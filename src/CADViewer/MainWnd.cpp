#include "pch.h"
#include "App.h"
#include "MainWnd.h"
#include "AboutDlg.h"
#include "gtl/qt/MatView/LayeredMatView.h"
#include "gtl/dxf/dxf.h"
#include "gtl/dwg/dwg.h"
#include "gtl/dwg/shape.h"
#include <sstream>
#include <cmath>
#include <opencv2/imgproc.hpp>

using namespace gtl::qt;

namespace {
QString PointText(gtl::shape::point_t const& p) {
    return QString("%1, %2, %3").arg(p.x,0,'g',14).arg(p.y,0,'g',14).arg(p.z,0,'g',14);
}
// Hit-test displayed strokes, not just bounding boxes (e.g. the empty center of a circle).
class SelectionCanvas : public gtl::shape::ICanvas {
public:
    QRectF region;
    bool any{}, inside{true}, crossing{};
    double tolerance;
    size_t segments{};
    SelectionCanvas(QRectF rect, double zoom) : region(rect), tolerance(.25 / std::max(zoom,1e-12)) {
        m_target_interpolation_inverval = tolerance;
    }
    void PreDraw(gtl::shape::xShape const&) override {}
    bool Contains(gtl::shape::point_t p) const {
        return p.x >= region.left() && p.x <= region.right() && p.y >= region.top() && p.y <= region.bottom();
    }
    void MoveTo_Target(gtl::shape::point_t const& p) override { m_ptLast=p; }
    void LineTo_Target(gtl::shape::point_t const& p) override {
        if (++segments > 100000) throw std::runtime_error("selection geometry limit");
        any=true; inside = inside && Contains(m_ptLast) && Contains(p);
        double lo=0,hi=1,dx=p.x-m_ptLast.x,dy=p.y-m_ptLast.y;
        auto clip=[&](double d,double q) {
            if(d==0) return q>=0;
            double t=q/d;
            if(d<0)lo=std::max(lo,t);else hi=std::min(hi,t);
            return lo<=hi;
        };
        if(clip(-dx,m_ptLast.x-region.left()) && clip(dx,region.right()-m_ptLast.x) &&
           clip(-dy,m_ptLast.y-region.top()) && clip(dy,region.bottom()-m_ptLast.y)) crossing=true;
        m_ptLast=p;
    }
    void Ellipse(gtl::shape::point_t const& c,double a,double b,gtl::deg_t axis,gtl::deg_t start,gtl::deg_t sweep) override {
        double r=std::max(std::abs(a),std::abs(b));
        double angle=static_cast<double>(gtl::rad_t(axis)), t0=static_cast<double>(gtl::rad_t(start)), length=static_cast<double>(gtl::rad_t(sweep));
        double step=r>tolerance ? 2*std::acos(std::clamp(1-tolerance/r,-1.,1.)) : .2;
        int n=static_cast<int>(std::clamp(std::ceil(std::abs(length)/std::max(step,1e-6)),16.,65536.));
        for(int i=0;i<=n;++i){double t=t0+length*i/n,x=a*std::cos(t),y=b*std::sin(t);
            gtl::shape::point_t p{c.x+x*std::cos(angle)-y*std::sin(angle),c.y+x*std::sin(angle)+y*std::cos(angle),c.z};
            if(i==0)MoveTo(p);else LineTo(p);
        }
    }
    void Arc(gtl::shape::point_t const& c,double radius,gtl::deg_t start,gtl::deg_t sweep) override {
        Ellipse(c,radius,radius,gtl::deg_t{0.},start,sweep);
    }
    void Spline(int degree,std::span<gtl::shape::point_t const> pts,std::span<double const> knots,bool loop) override {
        double length=0;for(size_t i=1;i<pts.size();++i)length+=pts[i].Distance(pts[i-1]);
        m_target_interpolation_inverval=std::max(tolerance,length/65536.);
        gtl::shape::ICanvas::Spline(degree,pts,knots,loop);
    }
    void Text(gtl::shape::xText const& text) override {
        // Match LayeredMatView's current horizontal OpenCV text footprint.
        double zoom=.25/tolerance;int baseline{};
        auto size=cv::getTextSize(QString::fromStdWString(text.m_text).toStdString(),cv::FONT_HERSHEY_SIMPLEX,
            std::max(.25,text.m_height*zoom/30.),std::max(1,text.m_lineWeight),&baseline);
        QRectF box(text.m_pt0.x,text.m_pt0.y-baseline/zoom,size.width/zoom,(size.height+baseline)/zoom);
        any=true;inside=inside && region.contains(box);crossing=crossing || region.intersects(box);
    }
    void Text(gtl::shape::xMText const& text) override { Text(static_cast<gtl::shape::xText const&>(text)); }
};
struct CursorGuard {
    CursorGuard() { QApplication::setOverrideCursor(Qt::WaitCursor); }
    ~CursorGuard() { QApplication::restoreOverrideCursor(); }
};
}

xMainWnd::xMainWnd(QWidget* parent) : base_t(parent) {
    ui.setupUi(this);
    LoadWindowPosition(theApp->GetReg(), "MainWnd", this);
    ui.drawingView->SetDrawPixelValue(false);
    ui.drawingView->installEventFilter(this);
    // Runtime sizing policies not exposed as Designer widget properties.
    ui.entityTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.propertyTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    resizeDocks({ui.entityDock, ui.propertyDock}, {280, 340}, Qt::Horizontal);
    connect(ui.actionOpen, &QAction::triggered, this, [this] {
        auto name = QFileDialog::getOpenFileName(this, tr("Open drawing"), {}, tr("CAD drawings (*.dxf *.dwg);;DXF (*.dxf);;DWG (*.dwg)"));
        if (!name.isEmpty()) OpenFile(name);
    });
    connect(ui.actionFit, &QAction::triggered, this, [this] { ui.drawingView->FitToWindow(); });
    connect(ui.actionClearSelection, &QAction::triggered, this, [this] { m_dragStart.reset(); ui.entityTree->clearSelection(); ui.entityTree->setCurrentItem(nullptr); ui.drawingView->ClearSelectionRect(); });
    connect(ui.entityTree, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item, int column) {
        if (column != 0 || !item->data(0, Qt::UserRole).isValid()) return;
        auto index = item->data(0, Qt::UserRole).toULongLong();
        if (index >= m_displayShapes.size()) return;
        m_displayShapes[index]->m_bVisible = item->checkState(0) == Qt::Checked;
        if (item->isSelected()) SelectEntity(item);
        ui.drawingView->update();
    });
    connect(ui.entityTree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* item, int) { FitTreeItem(item); });
    connect(ui.entityTree, &QTreeWidget::itemSelectionChanged, this, [this] { SelectEntity(ui.entityTree->currentItem()); });
    connect(ui.actionAbout, &QAction::triggered, this, &this_t::OnAction_About);
    statusBar()->showMessage(tr("Open a DXF or DWG drawing (Ctrl+O)."));
}

xMainWnd::~xMainWnd() {
    SaveWindowPosition(theApp->GetReg(), "MainWnd", this);
}

bool xMainWnd::OpenFile(QString const& filename) {
    QString error, notes;
    std::shared_ptr<gtl::shape::xDrawing> drawing;
    {
        CursorGuard cursor;
        try {
            auto path = std::filesystem::path(filename.toStdWString());
            auto ext = QFileInfo(filename).suffix().toLower();
            if (ext == "dwg") {
                gtl::dwg::xDWG source;
                if (!source.ReadDWG(path)) throw std::runtime_error(source.GetReport().message);
                gtl::dwg::sReadReport report;
                drawing = std::make_shared<gtl::shape::xDrawing>(gtl::dwg::ToShape(source, &report));
                if (!report.diagnostics.empty()) {
                    notes = tr("%1 diagnostics: some geometry may be omitted or approximated.").arg(report.diagnostics.size());
                    // Keep the complete report available without opening hundreds of modal dialogs.
                    for (auto const& d : report.diagnostics)
                        notes += QString("\nHandle %1, type %2: %3").arg(d.handle,0,16).arg(d.type).arg(QString::fromStdString(d.message));
                }
            } else if (ext == "dxf") {
                gtl::dxf::xDXF source;
                if (!source.ReadDXF(path)) throw std::runtime_error("Unable to read DXF file.");
                drawing = std::make_shared<gtl::shape::xDrawing>(gtl::dxf::ToShape(source));
            } else throw std::runtime_error("Unsupported file extension. Select a DXF or DWG file.");
        } catch (std::exception const& e) { error = QString::fromUtf8(e.what()); }
    }
    if (!drawing) {
        QMessageBox::warning(this, tr("Cannot open drawing"), filename + "\n\n" + error);
        return false;
    }
    // Commit only after loading succeeds, retaining the previous drawing on failure.
    ui.entityTree->clear();
    m_shapes.clear();
    m_displayShapes.clear();
    ui.drawingView->Clear();
    m_highlight = xLayeredMatView::npos;
    m_drawing = std::move(drawing);
    // xLayeredMatView draws lineWeight as pixels; CAD readers retain DWG/DXF weight codes.
    auto display = std::make_shared<gtl::shape::xDrawing>(*m_drawing);
    for (auto& layer : display->m_layers) {
        for (auto& shape : layer.m_shapes) {
            shape.m_lineWeight = std::clamp(static_cast<int>(std::lround(shape.GetLineWidthInUM() * 96. / 25400.)), 1, 8);
            // Capture initial effective visibility, then let each entity checkbox control it.
            shape.m_bVisible = display->m_bVisible && layer.m_bVisible && shape.m_bVisible;
            m_displayShapes.push_back(&shape);
        }
        layer.m_bVisible = true;
    }
    display->m_bVisible = true;
    ui.drawingView->Add(display);
    PopulateTree();
    ui.drawingView->FitToWindow();
    setWindowTitle(QFileInfo(filename).fileName() + " — CADViewer");
    auto summary = tr("%1 — %2 layers, %3 display entities").arg(filename).arg(m_drawing->m_layers.size()).arg(m_shapes.size());
    if (m_shapes.empty()) summary += tr(" — No displayable entities");
    if (!notes.isEmpty()) summary += " — " + notes.section('\n',0,0);
    statusBar()->showMessage(summary);
    ui.entityDetails->setPlainText(notes);
    ui.entityDetails->setToolTip(notes);
    return true;
}

void xMainWnd::PopulateTree() {
    QSignalBlocker block(ui.entityTree);
    ui.entityTree->setUpdatesEnabled(false);
    for (auto const& layer : m_drawing->m_layers) {
        auto* parent = new QTreeWidgetItem(ui.entityTree, {QString::fromStdWString(layer.m_name), tr("Layer (%1)").arg(layer.m_shapes.size())});
        // Qt propagates layer toggles to children and derives partial state from them.
        parent->setFlags(parent->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
        parent->setCheckState(0, layer.m_bVisible ? Qt::Checked : Qt::Unchecked);
        for (auto const& shape : layer.m_shapes) {
            auto type = QString::fromStdWString(shape.GetShapeName());
            auto* item = new QTreeWidgetItem(parent, {QString("%1  #%2").arg(type).arg(m_shapes.size()+1), type});
            item->setData(0, Qt::UserRole, QVariant::fromValue<qulonglong>(m_shapes.size()));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(0, m_displayShapes.at(m_shapes.size())->m_bVisible ? Qt::Checked : Qt::Unchecked);
            m_shapes.push_back(&shape);
        }
    }
    ui.entityTree->setUpdatesEnabled(true);
    ui.propertyTree->clear();
}

void xMainWnd::SelectEntity(QTreeWidgetItem* item) {
    if (m_highlight != xLayeredMatView::npos) ui.drawingView->Delete(m_highlight);
    m_highlight = xLayeredMatView::npos;
    ui.drawingView->ClearSelectionRect();
    ui.propertyTree->clear();
    ui.entityDetails->clear();
    auto items = ui.entityTree->selectedItems();
    auto overlay = std::make_shared<gtl::shape::xDrawing>();
    auto layer = std::make_unique<gtl::shape::xLayer>();
    QStringList descriptions;
    QRectF selectionBounds;
    bool hasBounds = false;
    for (auto* item : items) {
    if (!item->data(0, Qt::UserRole).isValid()) continue;
    auto index = item->data(0, Qt::UserRole).toULongLong();
    if (index >= m_shapes.size()) continue;
    auto const& shape = *m_shapes[index];
    QTreeWidgetItem* group = items.size() > 1 ? new QTreeWidgetItem(ui.propertyTree, {tr("Entity #%1").arg(index+1), QString::fromStdWString(shape.GetShapeName())}) : nullptr;
    auto add = [&](QString key, QString value) {
        if (group) new QTreeWidgetItem(group, {key,value});
        else new QTreeWidgetItem(ui.propertyTree, {key,value});
    };
    add(tr("Entity"), QString::number(index+1));
    add(tr("Type"), QString::fromStdWString(shape.GetShapeName()));
    add(tr("Layer"), item->parent()->text(0));
    add(tr("Color"), QColor(shape.m_color.r,shape.m_color.g,shape.m_color.b).name());
    add(tr("Visible"), m_displayShapes[index]->m_bVisible ? tr("Yes") : tr("No"));
    add(tr("Source visible"), shape.m_bVisible ? tr("Yes") : tr("No"));
    add(tr("Line type"), QString::fromStdWString(shape.m_strLineType));
    add(tr("Line weight"), QString::number(shape.m_lineWeight));
    if (auto ends = shape.GetStartEndPoint()) { add(tr("Start"),PointText(ends->first));add(tr("End"),PointText(ends->second)); }
    if (auto text = dynamic_cast<gtl::shape::xText const*>(&shape)) {
        add(tr("Text"),QString::fromStdWString(text->m_text));add(tr("Height"),QString::number(text->m_height,'g',14));
    }
    auto bounds = shape.GetBoundary();
    bool bounded = std::isfinite(bounds.pt0().x) && std::isfinite(bounds.pt0().y) && std::isfinite(bounds.pt1().x) && std::isfinite(bounds.pt1().y)
        && bounds.pt0().x <= bounds.pt1().x && bounds.pt0().y <= bounds.pt1().y;
    if (bounded) { add(tr("Bounds min"), PointText(bounds.pt0()));add(tr("Bounds max"),PointText(bounds.pt1())); }
    std::wostringstream details;
    shape.PrintOut(details);
    descriptions.push_back(tr("Entity #%1\n").arg(index+1) + QString::fromStdWString(details.str()));

    if (!m_displayShapes[index]->m_bVisible) continue;

    auto selected = shape.NewClone();
    selected->m_color = gtl::ColorRGBA(255,220,40);
    selected->m_lineWeight = 3;
    selected->m_bVisible = true;
    selected->m_bTransparent = false;
    layer->m_shapes.push_back(selected.release());
    if (bounded) {
        QRectF rect(QPointF(bounds.pt0().x,-bounds.pt1().y),QPointF(bounds.pt1().x,-bounds.pt0().y));
        if (!hasBounds) selectionBounds = rect;
        else { selectionBounds.setLeft(std::min(selectionBounds.left(),rect.left())); selectionBounds.setTop(std::min(selectionBounds.top(),rect.top())); selectionBounds.setRight(std::max(selectionBounds.right(),rect.right())); selectionBounds.setBottom(std::max(selectionBounds.bottom(),rect.bottom())); }
        hasBounds = true;
    }
    }
    ui.entityDetails->setPlainText(descriptions.join("\n"));
    if (!layer->m_shapes.empty()) {
        overlay->m_layers.push_back(layer.release());
        m_highlight = ui.drawingView->Add(overlay);
    }
    if (hasBounds) {
        double pad = 5. / std::max(ui.drawingView->Zoom(),1e-12);
        ui.drawingView->SetSelectionRect(gtl::xRect2d{selectionBounds.left()-pad,selectionBounds.top()-pad,selectionBounds.right()+pad,selectionBounds.bottom()+pad});
    }
}

void xMainWnd::FitTreeItem(QTreeWidgetItem* item) {
    if (!item) return;
    std::optional<QRectF> bounds;
    auto include = [&](QTreeWidgetItem* entity) {
        if (!entity->data(0,Qt::UserRole).isValid()) return;
        auto index=entity->data(0,Qt::UserRole).toULongLong();
        if(index>=m_shapes.size() || !m_displayShapes[index]->m_bVisible) return;
        auto r=m_shapes[index]->GetBoundary();
        if(!std::isfinite(r.pt0().x) || !std::isfinite(r.pt0().y) || !std::isfinite(r.pt1().x) || !std::isfinite(r.pt1().y)
            || r.pt0().x>r.pt1().x || r.pt0().y>r.pt1().y) return;
        QRectF rect(QPointF(r.pt0().x,-r.pt1().y),QPointF(r.pt1().x,-r.pt0().y));
        if(!bounds) bounds=rect;
        else { bounds->setLeft(std::min(bounds->left(),rect.left()));bounds->setTop(std::min(bounds->top(),rect.top()));
            bounds->setRight(std::max(bounds->right(),rect.right()));bounds->setBottom(std::max(bounds->bottom(),rect.bottom())); }
    };
    if(item->data(0,Qt::UserRole).isValid()) include(item);
    else for(int i=0;i<item->childCount();++i) include(item->child(i));
    if(!bounds) return;
    // Match LayeredMatView's drawing viewport, excluding its embedded toolbar.
    auto viewport=ui.drawingView->rect();
    if(auto* toolbar=ui.drawingView->findChild<QWidget*>("toolbar");toolbar && toolbar->isVisible())
        viewport.setTop(toolbar->geometry().bottom()+1);
    double zoom=ui.drawingView->Zoom();
    double width=bounds->width(),height=bounds->height();
    if(width>0 || height>0) {
        double zx=width>0 ? std::max(1,viewport.width()-40)/width : std::numeric_limits<double>::infinity();
        double zy=height>0 ? std::max(1,viewport.height()-40)/height : std::numeric_limits<double>::infinity();
        zoom=std::clamp(std::min(zx,zy),1./8192.,1000.);
    }
    ui.drawingView->SetZoom(zoom);
    auto center=bounds->center();
    ui.drawingView->SetPan({-center.x()*ui.drawingView->Zoom(),-center.y()*ui.drawingView->Zoom()});
    SelectEntity(ui.entityTree->currentItem());
}

bool xMainWnd::eventFilter(QObject* watched,QEvent* event) {
    if (watched != ui.drawingView) return base_t::eventFilter(watched,event);
    if(event->type()==QEvent::MouseButtonPress){
        auto* mouse=static_cast<QMouseEvent*>(event);
        if(mouse->button()==Qt::LeftButton || mouse->button()==Qt::RightButton){m_dragStart=mouse->position();ui.drawingView->setFocus();return true;}
    }
    if(event->type()==QEvent::MouseMove && m_dragStart){
        auto a=ui.drawingView->MapToWorld({m_dragStart->x(),m_dragStart->y()});
        auto pos=static_cast<QMouseEvent*>(event)->position();auto b=ui.drawingView->MapToWorld({pos.x(),pos.y()});
        gtl::xRect2d rect(a,b);rect.NormalizeRect();ui.drawingView->SetSelectionRect(rect);return true;
    }
    if(event->type()==QEvent::MouseButtonRelease && m_dragStart){
        auto* mouse=static_cast<QMouseEvent*>(event);
        if(mouse->button()==Qt::LeftButton || mouse->button()==Qt::RightButton){
            auto a=ui.drawingView->MapToWorld({m_dragStart->x(),m_dragStart->y()});
            auto b=ui.drawingView->MapToWorld({mouse->position().x(),mouse->position().y()});
            m_dragStart.reset();SelectRegion({a.x,a.y},{b.x,b.y});return true;
        }
    }
    if(event->type()==QEvent::KeyPress && static_cast<QKeyEvent*>(event)->key()==Qt::Key_Escape){
        m_dragStart.reset();ui.entityTree->clearSelection();ui.drawingView->ClearSelectionRect();return true;
    }
    return base_t::eventFilter(watched,event);
}

void xMainWnd::SelectRegion(QPointF start,QPointF end) {
    bool window=start.x()<=end.x();
    QRectF region(QPointF(std::min(start.x(),end.x()),-std::max(start.y(),end.y())),
                  QPointF(std::max(start.x(),end.x()),-std::min(start.y(),end.y())));
    size_t skipped=0;QTreeWidgetItem* first{};
    {
        QSignalBlocker block(ui.entityTree);
        ui.entityTree->clearSelection();ui.entityTree->setCurrentItem(nullptr);
        for(QTreeWidgetItemIterator it(ui.entityTree);*it;++it){
            auto* item=*it;if(!item->data(0,Qt::UserRole).isValid())continue;
            auto index=item->data(0,Qt::UserRole).toULongLong();
            if(index>=m_displayShapes.size() || !m_displayShapes[index]->m_bVisible)continue;
            try {
                SelectionCanvas hit(region,ui.drawingView->Zoom());m_displayShapes[index]->Draw(hit);
                if(hit.any && (window?hit.inside:hit.crossing)){
                    item->setSelected(true);item->parent()->setExpanded(true);if(!first)first=item;
                }
            }catch(std::exception const&){++skipped;}
        }
        if(first)ui.entityTree->setCurrentItem(first,0,QItemSelectionModel::NoUpdate);
    }
    SelectEntity(first);
    if(first)ui.entityTree->scrollToItem(first);
    statusBar()->showMessage(tr("%1 selection: %2 entities (%3 could not be tested)")
        .arg(window?tr("Window"):tr("Crossing")).arg(ui.entityTree->selectedItems().size()).arg(skipped));
}

void xMainWnd::OnAction_About(bool) {
    xAboutDlg dlg(this);
    dlg.exec();
}

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
#include <QSaveFile>
#include <QCryptographicHash>
#include <QMimeData>

#include "ShapeFile.inc"

using namespace gtl::qt;

namespace {
QString PointText(gtl::shape::point_t const& p) {
    return QString("%1, %2, %3").arg(p.x,0,'g',14).arg(p.y,0,'g',14).arg(p.z,0,'g',14);
}
struct EndpointDirections {
    gtl::shape::point_t start,end,startDir,endDir;
    bool closed{};
};
std::optional<EndpointDirections> GetEndpointDirections(gtl::shape::xShape const& shape) {
    using namespace gtl::shape;
    auto ends=shape.GetStartEndPoint();if(!ends)return {};
    EndpointDirections result{ends->first,ends->second,{},{},ends->first.Distance(ends->second)<1e-9};
    auto unit=[](point_t p){double length=std::hypot(p.x,p.y);return length>1e-12?point_t{p.x/length,p.y/length,0}:point_t{};};
    if(auto const* circle=dynamic_cast<xCircle const*>(&shape)){
        double start=0,sweep=static_cast<double>(gtl::rad_t(circle->m_angle_length)),rotation=0,rx=circle->m_radius,ry=rx;
        if(auto const* arc=dynamic_cast<xArc const*>(circle))start=static_cast<double>(gtl::rad_t(arc->m_angle_start));
        if(auto const* ellipse=dynamic_cast<xEllipse const*>(circle)){ry=ellipse->m_radiusH;rotation=static_cast<double>(gtl::rad_t(ellipse->m_angle_first_axis));}
        auto tangent=[&](double t){double c=std::cos(rotation),s=std::sin(rotation),sign=sweep<0?-1.:1.;return unit({sign*(-rx*std::sin(t)*c-ry*std::cos(t)*s),sign*(-rx*std::sin(t)*s+ry*std::cos(t)*c),0});};
        result.startDir=tangent(start);result.endDir=tangent(start+sweep);
    }else if(auto const* poly=dynamic_cast<xPolyline const*>(&shape)){
        auto n=poly->m_pts.size();if(n<2)return {};result.closed=poly->m_bLoop;
        auto tangent=[&](size_t i,bool end){auto const& a=poly->m_pts[i];auto const& b=poly->m_pts[(i+1)%n];
            if(a.Bulge()==0)return unit(point_t(b)-point_t(a));
            auto arc=xArc::GetFromBulge(a.Bulge(),a,b);double angle=static_cast<double>(gtl::rad_t(arc.m_angle_start+(end?arc.m_angle_length:gtl::deg_t{})));double sign=a.Bulge()<0?-1.:1.;return unit({-sign*std::sin(angle),sign*std::cos(angle),0});};
        for(size_t i=0;i<(poly->m_bLoop?n:n-1);++i){result.startDir=tangent(i,false);if(std::hypot(result.startDir.x,result.startDir.y)>0)break;}
        for(size_t i=poly->m_bLoop?n:n-1;i>0;--i){result.endDir=tangent(i-1,true);if(std::hypot(result.endDir.x,result.endDir.y)>0)break;}
    }else if(auto const* spline=dynamic_cast<xSpline const*>(&shape)){
        auto const& pts=spline->m_ptsControl;if(pts.size()<2)return {};
        for(size_t i=1;i<pts.size();++i){result.startDir=unit(pts[i]-pts.front());if(std::hypot(result.startDir.x,result.startDir.y)>0)break;}
        for(size_t i=pts.size()-1;i>0;--i){result.endDir=unit(pts.back()-pts[i-1]);if(std::hypot(result.endDir.x,result.endDir.y)>0)break;}
        result.closed=result.closed || (spline->m_flags&1)!=0;
    }else if(dynamic_cast<xLine const*>(&shape))result.startDir=result.endDir=unit(result.end-result.start);
    else return {}; // Points, text and area entities have no path direction.
    if(result.closed){result.end=result.start;result.endDir=result.startDir;}
    if(!std::isfinite(result.start.x) || !std::isfinite(result.start.y) || !std::isfinite(result.end.x) || !std::isfinite(result.end.y)
        || !std::isfinite(result.startDir.x) || !std::isfinite(result.endDir.x)
        || std::hypot(result.startDir.x,result.startDir.y)==0 || std::hypot(result.endDir.x,result.endDir.y)==0)return {};
    return result;
}
// Hit-test displayed strokes, not just bounding boxes (e.g. the empty center of a circle).
class xSelectionCanvas : public gtl::shape::ICanvas {
public:
    QRectF region;
    bool any{}, inside{true}, crossing{};
    double tolerance;
    size_t segments{};
    xSelectionCanvas(QRectF rect, double zoom) : region(rect), tolerance(.25 / std::max(zoom,1e-12)) {
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
    m_ui.setupUi(this);
    m_operationsDialog=new QDialog(this);m_operationsUi.setupUi(m_operationsDialog);
    m_filterDialog=new QDialog(this);m_filterUi.setupUi(m_filterDialog);
    for(auto* dialog:{m_operationsDialog,m_filterDialog}){
        dialog->installEventFilter(this);
        dialog->restoreGeometry(theApp->GetReg().value("CADViewer/"+dialog->objectName()+"/geometry").toByteArray());
    }
    LoadWindowPosition(theApp->GetReg(), "MainWnd", this);
    m_ui.drawingView->SetDrawPixelValue(false);
    m_ui.drawingView->installEventFilter(this);
    // Runtime sizing policies not exposed as Designer widget properties.
    m_ui.entityTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_ui.propertyTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    resizeDocks({m_ui.entityDock, m_ui.propertyDock}, {280, 340}, Qt::Horizontal);
    connect(m_ui.actionOpen, &QAction::triggered, this, [this] {
        auto name = QFileDialog::getOpenFileName(this, tr("Open drawing"), {}, tr("Drawings (*.dxf *.dwg *.shape);;Shape (*.shape);;DXF (*.dxf);;DWG (*.dwg)"));
        if (!name.isEmpty()) OpenFile(name);
    });
    connect(m_ui.actionFit, &QAction::triggered, this, [this] { m_ui.drawingView->FitToWindow(); });
    connect(m_ui.actionClearSelection, &QAction::triggered, this, [this] { m_isPickingPoint=false; m_dragStart.reset(); m_ui.entityTree->clearSelection(); m_ui.entityTree->setCurrentItem(nullptr); m_ui.drawingView->ClearSelectionRect(); });
    connect(m_ui.entityTree, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item, int column) {
        if (column != 0 || !item->data(0, Qt::UserRole).isValid()) return;
        auto index = item->data(0, Qt::UserRole).toULongLong();
        if (index >= m_displayShapes.size()) return;
        m_displayShapes[index]->m_bVisible = item->checkState(0) == Qt::Checked;
        if (item->checkState(0)==Qt::Checked) m_hiddenIds.erase(m_entityIds[index]); else m_hiddenIds.insert(m_entityIds[index]);
        if(!m_ui.actionShowText->isChecked() && dynamic_cast<gtl::shape::xText*>(m_displayShapes[index]))m_displayShapes[index]->m_bVisible=false;
        if (item->isSelected()) RefreshSelectionPresentation();
        m_ui.drawingView->update();
    });
    connect(m_ui.entityTree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* item, int) { FitTreeItem(item); });
    connect(m_ui.entityTree, &QTreeWidget::itemSelectionChanged, this, [this] { RefreshSelectionPresentation(); });
    connect(m_ui.actionAbout, &QAction::triggered, this, &this_t::OnActionAbout);
    SetupEditor();
    connect(m_ui.drawingView,&xLayeredMatView::ViewChanged,this,[this]{
        if(m_ui.actionDirections->isChecked() && m_selectionOverlayIndex!=xLayeredMatView::npos && !m_dragStart && !m_ui.entityTree->selectedItems().empty())RefreshSelectionPresentation();
    });
    statusBar()->showMessage(tr("Open a DXF, DWG or .shape drawing (Ctrl+O)."));
}

xMainWnd::~xMainWnd() {
    for(auto* dialog:{m_operationsDialog,m_filterDialog})theApp->GetReg().setValue("CADViewer/"+dialog->objectName()+"/geometry",dialog->saveGeometry());
    SaveWindowPosition(theApp->GetReg(), "MainWnd", this);
    theApp->GetReg().setValue("CADViewer/layout",saveState());
}

bool xMainWnd::OpenFile(QString const& filename) {
    if (!ConfirmDiscard()) return false;
    QString error, notes;
    std::shared_ptr<gtl::shape::xDrawing> drawing;
    {
        CursorGuard cursor;
        try {
            auto path = std::filesystem::path(filename.toStdWString());
            auto ext = QFileInfo(filename).suffix().toLower();
            if (ext == "shape") {
                drawing = ReadShapeFile(filename);
            } else if (ext == "dwg") {
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
            } else throw std::runtime_error("Unsupported file extension. Select DXF, DWG or .shape.");
        } catch (std::exception const& e) { error = QString::fromUtf8(e.what()); }
    }
    if (!drawing) {
        QMessageBox::warning(this, tr("Cannot open drawing"), filename + "\n\n" + error);
        return false;
    }
    // Commit only after loading succeeds, retaining the previous drawing on failure.
    m_drawing = std::move(drawing);
    m_entityIds.clear();m_workingSetIds.clear();m_hiddenIds.clear();m_history.clear();m_nextEntityId=1;
    for(auto const& layer:m_drawing->m_layers)for(auto const& shape:layer.m_shapes){
        m_entityIds.push_back(m_nextEntityId++);
        if(!m_drawing->m_bVisible || !layer.m_bVisible || !shape.m_bVisible)m_hiddenIds.insert(m_entityIds.back());
    }
    m_filePath=filename;m_revision=m_nextRevision++;m_savedRevision=m_revision;
    RefreshDrawing();PushHistory(tr("Opened drawing"));
    m_ui.drawingView->FitToWindow();
    setWindowTitle(QFileInfo(filename).fileName() + " — CADViewer");
    auto summary = tr("%1 — %2 layers, %3 display entities").arg(filename).arg(m_drawing->m_layers.size()).arg(m_sourceShapes.size());
    if (m_sourceShapes.empty()) summary += tr(" — No displayable entities");
    if (!notes.isEmpty()) summary += " — " + notes.section('\n',0,0);
    Log(summary);
    if(!notes.isEmpty())m_ui.logText->appendPlainText(notes);
    m_ui.entityDetails->setPlainText(notes);
    m_ui.entityDetails->setToolTip(notes);
    return true;
}

void xMainWnd::PopulateTree() {
    QSignalBlocker block(m_ui.entityTree);
    m_ui.entityTree->setUpdatesEnabled(false);
    for (auto const& layer : m_drawing->m_layers) {
        auto* parent = new QTreeWidgetItem(m_ui.entityTree, {QString::fromStdWString(layer.m_name), tr("Layer (%1)").arg(layer.m_shapes.size())});
        // Qt propagates layer toggles to children and derives partial state from them.
        parent->setFlags(parent->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
        parent->setCheckState(0, layer.m_bVisible ? Qt::Checked : Qt::Unchecked);
        for (auto const& shape : layer.m_shapes) {
            auto type = QString::fromStdWString(shape.GetShapeName());
            auto* item = new QTreeWidgetItem(parent, {QString("%1  #%2").arg(type).arg(m_entityIds.at(m_sourceShapes.size())), type});
            item->setData(0, Qt::UserRole, QVariant::fromValue<qulonglong>(m_sourceShapes.size()));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(0, !m_hiddenIds.contains(m_entityIds.at(m_sourceShapes.size())) ? Qt::Checked : Qt::Unchecked);
            m_sourceShapes.push_back(&shape);
        }
    }
    m_ui.entityTree->setUpdatesEnabled(true);
    m_ui.propertyTree->clear();
}

void xMainWnd::RefreshSelectionPresentation() {
    if (m_selectionOverlayIndex != xLayeredMatView::npos) m_ui.drawingView->Delete(m_selectionOverlayIndex);
    m_selectionOverlayIndex = xLayeredMatView::npos;
    m_ui.drawingView->ClearSelectionRect();
    m_ui.propertyTree->clear();
    m_ui.entityDetails->clear();
    auto items = m_ui.entityTree->selectedItems();
    auto overlay = std::make_shared<gtl::shape::xDrawing>();
    auto layer = std::make_unique<gtl::shape::xLayer>();
    QStringList descriptions;
    for (auto* item : items) {
    if (!item->data(0, Qt::UserRole).isValid()) continue;
    auto index = item->data(0, Qt::UserRole).toULongLong();
    if (index >= m_sourceShapes.size()) continue;
    auto const& shape = *m_sourceShapes[index];
    QTreeWidgetItem* group = items.size() > 1 ? new QTreeWidgetItem(m_ui.propertyTree, {tr("Entity #%1").arg(m_entityIds.at(index)), QString::fromStdWString(shape.GetShapeName())}) : nullptr;
    auto add = [&](QString key, QString value) {
        if (group) new QTreeWidgetItem(group, {key,value});
        else new QTreeWidgetItem(m_ui.propertyTree, {key,value});
    };
    add(tr("Entity ID"), QString::number(m_entityIds[index]));
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
    descriptions.push_back(tr("Entity #%1\n").arg(m_entityIds.at(index)) + QString::fromStdWString(details.str()));

    if (!m_displayShapes[index]->m_bVisible) continue;

    auto selected = shape.NewClone();
    selected->m_color = gtl::ColorRGBA(255,220,40);
    selected->m_lineWeight = std::max(3, m_displayShapes[index]->m_lineWeight + 2);
    selected->m_bVisible = true;
    selected->m_bTransparent = false;
    layer->m_shapes.push_back(selected.release());
    if(m_ui.actionDirections->isChecked())if(auto marks=GetEndpointDirections(shape)){
        double pixel=1./std::max(m_ui.drawingView->Zoom(),1e-12);
        auto arrow=[&](gtl::shape::point_t tip,gtl::shape::point_t direction,auto color){
            auto back=tip-direction*(11*pixel);
            gtl::shape::point_t normal{-direction.y*4.5*pixel,direction.x*4.5*pixel,0};
            auto triangle=std::make_unique<gtl::shape::xPolyline>();
            triangle->m_pts.emplace_back(tip);triangle->m_pts.emplace_back(back+normal);triangle->m_pts.emplace_back(back-normal);
            triangle->m_bLoop=true;triangle->m_color=color;triangle->m_lineWeight=2;
            layer->m_shapes.push_back(triangle.release());
        };
        arrow(marks->start,marks->startDir,gtl::ColorRGBA(40,220,100));
        auto end=marks->closed?marks->start+marks->startDir*(18*pixel):marks->end;
        arrow(end,marks->endDir,gtl::ColorRGBA(255,80,80));
    }

    }
    m_ui.entityDetails->setPlainText(descriptions.join("\n"));
    if (!layer->m_shapes.empty()) {
        overlay->m_layers.push_back(layer.release());
        m_selectionOverlayIndex = m_ui.drawingView->Add(overlay);
    }

}

void xMainWnd::FitTreeItem(QTreeWidgetItem* item) {
    if (!item) return;
    std::optional<QRectF> bounds;
    auto include = [&](QTreeWidgetItem* entity) {
        if (!entity->data(0,Qt::UserRole).isValid()) return;
        auto index=entity->data(0,Qt::UserRole).toULongLong();
        if(index>=m_sourceShapes.size() || !m_displayShapes[index]->m_bVisible) return;
        auto r=m_sourceShapes[index]->GetBoundary();
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
    auto viewport=m_ui.drawingView->rect();
    if(auto* toolbar=m_ui.drawingView->findChild<QWidget*>("toolbar");toolbar && toolbar->isVisible())
        viewport.setTop(toolbar->geometry().bottom()+1);
    double zoom=m_ui.drawingView->Zoom();
    double width=bounds->width(),height=bounds->height();
    if(width>0 || height>0) {
        double zx=width>0 ? std::max(1,viewport.width()-40)/width : std::numeric_limits<double>::infinity();
        double zy=height>0 ? std::max(1,viewport.height()-40)/height : std::numeric_limits<double>::infinity();
        zoom=std::clamp(std::min(zx,zy),1./8192.,1000.);
    }
    m_ui.drawingView->SetZoom(zoom);
    auto center=bounds->center();
    m_ui.drawingView->SetPan({-center.x()*m_ui.drawingView->Zoom(),-center.y()*m_ui.drawingView->Zoom()});
    RefreshSelectionPresentation();
}

bool xMainWnd::eventFilter(QObject* watched,QEvent* event) {
    if(watched==m_operationsDialog || watched==m_filterDialog){
        if(event->type()==QEvent::Show || event->type()==QEvent::Hide){
            auto* action=watched==m_operationsDialog?m_ui.actionOperations:m_ui.actionSelectionFilter;
            QSignalBlocker block(action);action->setChecked(event->type()==QEvent::Show);
            if(watched==m_operationsDialog && event->type()==QEvent::Hide)m_isPickingPoint=false;
        }
        return base_t::eventFilter(watched,event);
    }

    if (watched != m_ui.drawingView) return base_t::eventFilter(watched,event);
    if(event->type()==QEvent::MouseButtonPress){
        auto* mouse=static_cast<QMouseEvent*>(event);
        if(m_isPickingPoint && mouse->button()==Qt::LeftButton){
            auto world=m_ui.drawingView->MapToWorld({mouse->position().x(),mouse->position().y()});
            m_operationsUi.opX->setValue(world.x);m_operationsUi.opY->setValue(-world.y);m_isPickingPoint=false;statusBar()->showMessage(tr("Point picked"));return true;
        }
        if(mouse->button()==Qt::LeftButton || mouse->button()==Qt::RightButton){m_dragStart=mouse->position();m_ui.drawingView->setFocus();return true;}
    }
    if(event->type()==QEvent::MouseMove && m_dragStart){
        auto a=m_ui.drawingView->MapToWorld({m_dragStart->x(),m_dragStart->y()});
        auto pos=static_cast<QMouseEvent*>(event)->position();auto b=m_ui.drawingView->MapToWorld({pos.x(),pos.y()});
        gtl::xRect2d rect(a,b);rect.NormalizeRect();m_ui.drawingView->SetSelectionRect(rect);return true;
    }
    if(event->type()==QEvent::MouseButtonRelease && m_dragStart){
        auto* mouse=static_cast<QMouseEvent*>(event);
        if(mouse->button()==Qt::LeftButton || mouse->button()==Qt::RightButton){
            auto a=m_ui.drawingView->MapToWorld({m_dragStart->x(),m_dragStart->y()});
            auto b=m_ui.drawingView->MapToWorld({mouse->position().x(),mouse->position().y()});
            m_dragStart.reset();SelectRegion({a.x,a.y},{b.x,b.y});return true;
        }
    }
    if(event->type()==QEvent::KeyPress && static_cast<QKeyEvent*>(event)->key()==Qt::Key_Escape){
        m_isPickingPoint=false;m_dragStart.reset();m_ui.entityTree->clearSelection();m_ui.drawingView->ClearSelectionRect();return true;
    }
    return base_t::eventFilter(watched,event);
}

void xMainWnd::SelectRegion(QPointF start,QPointF end) {
    bool window=start.x()<=end.x();
    QRectF region(QPointF(std::min(start.x(),end.x()),-std::max(start.y(),end.y())),
                  QPointF(std::max(start.x(),end.x()),-std::min(start.y(),end.y())));
    size_t skipped=0;QTreeWidgetItem* first{};
    {
        QSignalBlocker block(m_ui.entityTree);
        m_ui.entityTree->clearSelection();m_ui.entityTree->setCurrentItem(nullptr);
        for(QTreeWidgetItemIterator it(m_ui.entityTree);*it;++it){
            auto* item=*it;if(!item->data(0,Qt::UserRole).isValid())continue;
            auto index=item->data(0,Qt::UserRole).toULongLong();
            if(index>=m_displayShapes.size() || !m_displayShapes[index]->m_bVisible || (m_filterUi.mouseFilter->isChecked() && !MatchesFilter(index)))continue;
            try {
                xSelectionCanvas hit(region,m_ui.drawingView->Zoom());m_displayShapes[index]->Draw(hit);
                if(hit.any && (window?hit.inside:hit.crossing)){
                    item->setSelected(true);item->parent()->setExpanded(true);if(!first)first=item;
                }
            }catch(std::exception const&){++skipped;}
        }
        if(first)m_ui.entityTree->setCurrentItem(first,0,QItemSelectionModel::NoUpdate);
    }
    RefreshSelectionPresentation();
    if(first)m_ui.entityTree->scrollToItem(first);
    statusBar()->showMessage(tr("%1 selection: %2 entities (%3 could not be tested)")
        .arg(window?tr("Window"):tr("Crossing")).arg(m_ui.entityTree->selectedItems().size()).arg(skipped));
}

void xMainWnd::OnActionAbout(bool) {
    xAboutDlg dlg(this);
    dlg.exec();
}

#include "Editor.inc"

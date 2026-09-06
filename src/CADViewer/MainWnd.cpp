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

using namespace gtl::qt;

namespace {
QString PointText(gtl::shape::point_t const& p) {
    return QString("%1, %2, %3").arg(p.x,0,'g',14).arg(p.y,0,'g',14).arg(p.z,0,'g',14);
}
struct CursorGuard {
    CursorGuard() { QApplication::setOverrideCursor(Qt::WaitCursor); }
    ~CursorGuard() { QApplication::restoreOverrideCursor(); }
};
}

xMainWnd::xMainWnd(QWidget* parent) : base_t(parent) {
    ui.setupUi(this);
    resize(1280, 800);
    LoadWindowPosition(theApp->GetReg(), "MainWnd", this);
    setWindowIcon(QIcon(":/image/icon.ico"));
    m_view = new xLayeredMatView(this);
    m_view->setObjectName("drawingView");
    m_view->SetDrawPixelValue(false);
    setCentralWidget(m_view);

    auto* treeDock = new QDockWidget(tr("Entities (display geometry)"), this);
    treeDock->setObjectName("entityDock");
    m_entities = new QTreeWidget(treeDock);
    m_entities->setObjectName("entityTree");
    m_entities->setHeaderLabels({tr("Layer / Entity"), tr("Type")});
    m_entities->setSelectionMode(QAbstractItemView::SingleSelection);
    m_entities->setUniformRowHeights(true);
    m_entities->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    treeDock->setWidget(m_entities);
    addDockWidget(Qt::LeftDockWidgetArea, treeDock);

    auto* propertyDock = new QDockWidget(tr("Properties"), this);
    propertyDock->setObjectName("propertyDock");
    auto* split = new QSplitter(Qt::Vertical, propertyDock);
    m_properties = new QTreeWidget(split);
    m_properties->setObjectName("propertyTree");
    m_properties->setHeaderLabels({tr("Property"), tr("Value")});
    m_properties->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_details = new QPlainTextEdit(split);
    m_details->setObjectName("entityDetails");
    m_details->setReadOnly(true);
    m_details->setPlaceholderText(tr("Select an entity to inspect its geometry."));
    propertyDock->setWidget(split);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock);
    resizeDocks({treeDock, propertyDock}, {280, 340}, Qt::Horizontal);

    auto* open = new QAction(tr("Open…"), this);
    open->setShortcut(QKeySequence::Open);
    ui.menuFiles->insertAction(ui.actionAbout, open);
    ui.mainToolBar->addAction(open);
    connect(open, &QAction::triggered, this, [this] {
        auto name = QFileDialog::getOpenFileName(this, tr("Open drawing"), {}, tr("CAD drawings (*.dxf *.dwg);;DXF (*.dxf);;DWG (*.dwg)"));
        if (!name.isEmpty()) OpenFile(name);
    });
    auto* fit = ui.mainToolBar->addAction(tr("Fit drawing"));
    fit->setShortcut(QKeySequence(Qt::Key_Home));
    connect(fit, &QAction::triggered, this, [this] { m_view->FitToWindow(); });
    auto* clear = ui.mainToolBar->addAction(tr("Clear selection"));
    clear->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(clear, &QAction::triggered, this, [this] { m_entities->setCurrentItem(nullptr); });
    auto* viewMenu = ui.menuBar->addMenu(tr("View"));
    viewMenu->addAction(treeDock->toggleViewAction());
    viewMenu->addAction(propertyDock->toggleViewAction());
    connect(m_entities, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* item) { SelectEntity(item); });
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
    m_entities->clear();
    m_shapes.clear();
    m_view->Clear();
    m_highlight = xLayeredMatView::npos;
    m_drawing = std::move(drawing);
    // xLayeredMatView draws lineWeight as pixels; CAD readers retain DWG/DXF weight codes.
    auto display = std::make_shared<gtl::shape::xDrawing>(*m_drawing);
    for (auto& layer : display->m_layers)
        for (auto& shape : layer.m_shapes)
            shape.m_lineWeight = std::clamp(static_cast<int>(std::lround(shape.GetLineWidthInUM() * 96. / 25400.)), 1, 8);
    m_view->Add(display);
    PopulateTree();
    m_view->FitToWindow();
    setWindowTitle(QFileInfo(filename).fileName() + " — CADViewer");
    auto summary = tr("%1 — %2 layers, %3 display entities").arg(filename).arg(m_drawing->m_layers.size()).arg(m_shapes.size());
    if (m_shapes.empty()) summary += tr(" — No displayable entities");
    if (!notes.isEmpty()) summary += " — " + notes.section('\n',0,0);
    statusBar()->showMessage(summary);
    m_details->setPlainText(notes);
    m_details->setToolTip(notes);
    return true;
}

void xMainWnd::PopulateTree() {
    QSignalBlocker block(m_entities);
    m_entities->setUpdatesEnabled(false);
    for (auto const& layer : m_drawing->m_layers) {
        auto* parent = new QTreeWidgetItem(m_entities, {QString::fromStdWString(layer.m_name), tr("Layer (%1)").arg(layer.m_shapes.size())});
        for (auto const& shape : layer.m_shapes) {
            auto type = QString::fromStdWString(shape.GetShapeName());
            auto* item = new QTreeWidgetItem(parent, {QString("%1  #%2").arg(type).arg(m_shapes.size()+1), type});
            item->setData(0, Qt::UserRole, QVariant::fromValue<qulonglong>(m_shapes.size()));
            m_shapes.push_back(&shape);
        }
    }
    m_entities->setUpdatesEnabled(true);
    m_properties->clear();
}

void xMainWnd::SelectEntity(QTreeWidgetItem* item) {
    if (m_highlight != xLayeredMatView::npos) m_view->Delete(m_highlight);
    m_highlight = xLayeredMatView::npos;
    m_view->ClearSelectionRect();
    m_properties->clear();
    m_details->clear();
    if (!item || !item->data(0, Qt::UserRole).isValid()) return;
    auto index = item->data(0, Qt::UserRole).toULongLong();
    if (index >= m_shapes.size()) return;
    auto const& shape = *m_shapes[index];
    auto add = [&](QString key, QString value) { new QTreeWidgetItem(m_properties, {key, value}); };
    add(tr("Entity"), QString::number(index+1));
    add(tr("Type"), QString::fromStdWString(shape.GetShapeName()));
    add(tr("Layer"), item->parent()->text(0));
    add(tr("Color"), QColor(shape.m_color.r,shape.m_color.g,shape.m_color.b).name());
    add(tr("Visible"), shape.m_bVisible ? tr("Yes") : tr("No"));
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
    m_details->setPlainText(QString::fromStdWString(details.str()));

    auto overlay = std::make_shared<gtl::shape::xDrawing>();
    auto layer = std::make_unique<gtl::shape::xLayer>();
    auto selected = shape.NewClone();
    selected->m_color = gtl::ColorRGBA(255,220,40);
    selected->m_lineWeight = 3;
    selected->m_bVisible = true;
    selected->m_bTransparent = false;
    layer->m_shapes.push_back(selected.release());
    overlay->m_layers.push_back(layer.release());
    m_highlight = m_view->Add(overlay);
    if (bounded) {
        // Drawings use CAD Y-up; the image view's world coordinates use Y-down.
        double pad = 5. / std::max(m_view->Zoom(),1e-12);
        m_view->SetSelectionRect(gtl::xRect2d{bounds.pt0().x-pad,-bounds.pt1().y-pad,bounds.pt1().x+pad,-bounds.pt0().y+pad});
    }
}

void xMainWnd::OnAction_About(bool) {
    xAboutDlg dlg(this);
    dlg.exec();
}

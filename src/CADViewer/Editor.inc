// Editor/controller implementation included by MainWnd.cpp (one Shape-registration TU).
namespace {
using Shape=gtl::shape::xShape;
using Drawing=gtl::shape::xDrawing;
struct EditRecord { std::uint64_t id; std::wstring layer; std::shared_ptr<Shape> shape; };
std::vector<EditRecord> CloneEntityRecords(Drawing const& drawing,std::vector<std::uint64_t> const& ids) {
    std::vector<EditRecord> result;size_t i=0;
    for(auto const& layer:drawing.m_layers)for(auto const& shape:layer.m_shapes)
        result.push_back({ids.at(i++),layer.m_name,std::shared_ptr<Shape>(shape.NewClone().release())});
    return result;
}
std::shared_ptr<Drawing> BuildDrawing(Drawing const& source,std::vector<EditRecord> const& records,std::vector<std::uint64_t>& ids) {
    auto drawing=std::make_shared<Drawing>(source);ids.clear();
    for(auto& layer:drawing->m_layers)layer.m_shapes.clear();
    for(auto const& record:records){
        auto layer=std::find_if(drawing->m_layers.begin(),drawing->m_layers.end(),[&](auto const& l){return l.m_name==record.layer;});
        if(layer==drawing->m_layers.end())drawing->m_layers.push_back(new gtl::shape::xLayer(record.layer));
    }
    for(auto& layer:drawing->m_layers)for(auto const& record:records)if(record.layer==layer.m_name){
        layer.m_shapes.push_back(record.shape->NewClone().release());ids.push_back(record.id);
    }
    return drawing;
}
std::unique_ptr<gtl::shape::xPolyline> AsPolyline(Shape const& shape) {
    using namespace gtl::shape;
    if(auto const* p=dynamic_cast<xPolyline const*>(&shape))return std::make_unique<xPolyline>(*p);
    auto p=std::make_unique<xPolyline>();static_cast<Shape&>(*p)=shape;
    if(auto const* line=dynamic_cast<xLine const*>(&shape)){p->m_pts.emplace_back(line->m_pt0);p->m_pts.emplace_back(line->m_pt1);}
    else if(shape.GetShapeType()==eSHAPE::arc_xy){
        auto const& arc=static_cast<xArc const&>(shape);auto ends=arc.GetStartEndPoint();
        if(!ends || std::abs(static_cast<double>(arc.m_angle_length))>=359.999999)throw std::runtime_error("Join requires open arcs");
        p->m_pts.emplace_back(ends->first);p->m_pts.back().Bulge()=std::tan(static_cast<double>(gtl::rad_t(arc.m_angle_length))/4.);p->m_pts.emplace_back(ends->second);
    }else throw std::runtime_error("Join supports LINE, ARC and open POLYLINE");
    return p;
}
}

#include "EditorGeometry.inc"

std::set<xMainWnd::EntityId> xMainWnd::SelectedIds() const {
    std::set<EntityId> ids;
    for(auto* item:m_ui.entityTree->selectedItems())if(item->data(0,Qt::UserRole).isValid()){
        auto i=item->data(0,Qt::UserRole).toULongLong();if(i<m_entityIds.size())ids.insert(m_entityIds[i]);
    }
    return ids;
}
void xMainWnd::SelectIds(std::set<EntityId> const& ids) {
    {QSignalBlocker block(m_ui.entityTree);m_ui.entityTree->clearSelection();QTreeWidgetItem* first{};
    for(QTreeWidgetItemIterator it(m_ui.entityTree);*it;++it){auto* item=*it;if(!item->data(0,Qt::UserRole).isValid())continue;
        auto i=item->data(0,Qt::UserRole).toULongLong();if(i<m_entityIds.size() && ids.contains(m_entityIds[i])){item->setSelected(true);item->parent()->setExpanded(true);if(!first)first=item;}}
    m_ui.entityTree->setCurrentItem(first,0,QItemSelectionModel::NoUpdate);}
    RefreshSelectionPresentation();
}
void xMainWnd::RefreshDrawing(std::set<EntityId> const& selected) {
    auto zoom=m_ui.drawingView->Zoom();auto pan=m_ui.drawingView->Pan();
    {QSignalBlocker blocker(m_ui.entityTree);m_ui.entityTree->clear();}
    m_sourceShapes.clear();m_displayShapes.clear();m_ui.drawingView->Clear();m_selectionOverlayIndex=xLayeredMatView::npos;
    if(!m_drawing)return;
    auto display=std::make_shared<Drawing>(*m_drawing);size_t i=0;
    for(auto& layer:display->m_layers){
        for(auto& shape:layer.m_shapes){
            shape.m_lineWeight=std::clamp(static_cast<int>(std::lround(shape.GetLineWidthInUM()*96./25400.)),1,8);
            shape.m_bVisible=!m_hiddenIds.contains(m_entityIds.at(i++));
            if(!m_ui.actionShowText->isChecked() && dynamic_cast<gtl::shape::xText*>(&shape))shape.m_bVisible=false;
            if(!m_ui.actionDark->isChecked() && shape.m_color.r>230 && shape.m_color.g>230 && shape.m_color.b>230)shape.m_color=gtl::ColorRGBA(20,20,20);
            m_displayShapes.push_back(&shape);
        }layer.m_bVisible=true;
    }display->m_bVisible=true;
    m_ui.drawingView->Add(display);PopulateTree();
    m_ui.drawingView->SetZoom(zoom);m_ui.drawingView->SetPan(pan);
    SelectIds(selected);RefreshEditorPanels();
}
xMainWnd::EditState xMainWnd::CaptureState(QString label) const {
    return {std::make_shared<Drawing>(*m_drawing),m_entityIds,m_workingSetIds,m_hiddenIds,SelectedIds(),m_revision,label};
}
void xMainWnd::PushHistory(QString label) {
    if(!m_history.empty())m_history.resize(m_historyPosition+1);
    m_history.push_back(CaptureState(label));
    if(m_history.size()>51)m_history.erase(m_history.begin());
    m_historyPosition=m_history.size()-1;RefreshEditorPanels();
}
void xMainWnd::RestoreHistory(size_t position) {
    if(position>=m_history.size() || position==m_historyPosition)return;
    m_history[m_historyPosition]=CaptureState(m_history[m_historyPosition].label);
    auto const& state=m_history[position];m_drawing=std::make_shared<Drawing>(*state.drawing);m_entityIds=state.entityIds;
    m_workingSetIds=state.workingSetIds;m_hiddenIds=state.hiddenIds;m_revision=state.revision;m_historyPosition=position;
    RefreshDrawing(state.selectedIds);Log(tr("History: %1").arg(state.label));
}
void xMainWnd::RefreshEditorPanels() {
    QSignalBlocker block(m_ui.historyList);m_ui.historyList->clear();
    for(auto const& state:m_history)m_ui.historyList->addItem(state.label);
    if(!m_history.empty())m_ui.historyList->setCurrentRow(static_cast<int>(m_historyPosition));
    m_ui.actionUndo->setEnabled(m_historyPosition>0);m_ui.actionRedo->setEnabled(m_historyPosition+1<m_history.size());
    m_ui.workingList->clear();for(size_t i=0;i<m_entityIds.size();++i)if(m_workingSetIds.contains(m_entityIds[i])){
        auto* item=new QListWidgetItem(tr("#%1 %2").arg(m_entityIds[i]).arg(QString::fromStdWString(m_sourceShapes[i]->GetShapeName())),m_ui.workingList);
        item->setData(Qt::UserRole,QVariant::fromValue<qulonglong>(m_entityIds[i]));
    }
    auto type=m_filterUi.filterType->currentText(),layer=m_filterUi.filterLayer->currentText();
    QSignalBlocker types(m_filterUi.filterType),layers(m_filterUi.filterLayer);m_filterUi.filterType->clear();m_filterUi.filterLayer->clear();
    m_filterUi.filterType->addItem(tr("All types"));m_filterUi.filterLayer->addItem(tr("All layers"));
    std::set<QString> names;for(auto* shape:m_sourceShapes)names.insert(QString::fromStdWString(shape->GetShapeName()));
    for(auto const& name:names)m_filterUi.filterType->addItem(name);
    if(m_drawing)for(auto const& l:m_drawing->m_layers)m_filterUi.filterLayer->addItem(QString::fromStdWString(l.m_name));
    m_filterUi.filterType->setCurrentIndex(std::max(0,m_filterUi.filterType->findText(type)));m_filterUi.filterLayer->setCurrentIndex(std::max(0,m_filterUi.filterLayer->findText(layer)));
    setWindowTitle((m_filePath.isEmpty()?tr("Untitled"):QFileInfo(m_filePath).fileName())+(m_revision!=m_savedRevision?" *":"")+" — CADViewer");
}
bool xMainWnd::MatchesFilter(size_t index) const {
    if(index>=m_sourceShapes.size())return false;
    if(m_filterUi.filterType->currentIndex()>0 && QString::fromStdWString(m_sourceShapes[index]->GetShapeName())!=m_filterUi.filterType->currentText())return false;
    if(m_filterUi.filterLayer->currentIndex()>0){size_t i=0;bool match=false;for(auto const& layer:m_drawing->m_layers){if(index>=i && index<i+layer.m_shapes.size()){match=QString::fromStdWString(layer.m_name)==m_filterUi.filterLayer->currentText();break;}i+=layer.m_shapes.size();}if(!match)return false;}
    auto bounds=m_sourceShapes[index]->GetBoundary();double size=std::max(bounds.Width(),bounds.Height());
    return std::isfinite(size) && size>=m_filterUi.filterMin->value() && (m_filterUi.filterMax->value()==0 || size<=m_filterUi.filterMax->value());
}
void xMainWnd::Log(QString const& text) {m_ui.logText->appendPlainText(text);statusBar()->showMessage(text);}
bool xMainWnd::SaveShape(QString const& filename) {
    if(!m_drawing)return false;
    try{auto path=filename;if(!path.endsWith(".shape",Qt::CaseInsensitive))path+=".shape";WriteShapeFile(path,*m_drawing);m_filePath=path;m_savedRevision=m_revision;RefreshEditorPanels();Log(tr("Saved %1").arg(m_filePath));return true;}
    catch(std::exception const& e){Log(QString::fromUtf8(e.what()));QMessageBox::warning(this,tr("Save failed"),QString::fromUtf8(e.what()));return false;}
}
void xMainWnd::ExportShapes(QString filename,std::set<EntityId> const& ids) {
    if(!m_drawing || ids.empty())return;
    auto records=CloneEntityRecords(*m_drawing,m_entityIds);std::erase_if(records,[&](auto const& r){return !ids.contains(r.id);});std::vector<EntityId> ignored;
    try{WriteShapeFile(filename,*BuildDrawing(*m_drawing,records,ignored));Log(tr("Exported %1 entities").arg(records.size()));}
    catch(std::exception const& e){QMessageBox::warning(this,tr("Export failed"),QString::fromUtf8(e.what()));}
}
bool xMainWnd::ConfirmDiscard() {
    if(!m_drawing || m_revision==m_savedRevision)return true;
    auto choice=QMessageBox::question(this,tr("Unsaved drawing"),tr("Save changes to .shape?"),QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
    if(choice==QMessageBox::Cancel)return false;if(choice==QMessageBox::Discard)return true;
    auto path=m_filePath;if(!path.endsWith(".shape",Qt::CaseInsensitive))path=QFileDialog::getSaveFileName(this,tr("Save drawing"),QFileInfo(path).completeBaseName()+".shape",tr("Shape (*.shape)"));
    if(path.isEmpty())return false;if(!path.endsWith(".shape",Qt::CaseInsensitive))path+=".shape";return SaveShape(path);
}
void xMainWnd::closeEvent(QCloseEvent* event){if(ConfirmDiscard())base_t::closeEvent(event);else event->ignore();}
void xMainWnd::dragEnterEvent(QDragEnterEvent* event){if(event->mimeData()->hasUrls() && event->mimeData()->urls().front().isLocalFile())event->acceptProposedAction();}
void xMainWnd::dropEvent(QDropEvent* event){if(event->mimeData()->hasUrls()){OpenFile(event->mimeData()->urls().front().toLocalFile());event->acceptProposedAction();}}
namespace {
bool IsEditingText() {auto* w=QApplication::focusWidget();return qobject_cast<QLineEdit*>(w) || qobject_cast<QTextEdit*>(w) || qobject_cast<QPlainTextEdit*>(w) || qobject_cast<QAbstractSpinBox*>(w);}

}
void xMainWnd::CopySelection(){
    if(IsEditingText())return;
    if(!m_drawing)return;auto ids=SelectedIds();auto records=CloneEntityRecords(*m_drawing,m_entityIds);std::erase_if(records,[&](auto const& r){return !ids.contains(r.id);});std::vector<EntityId> ignored;
    m_clipboard=BuildDrawing(*m_drawing,records,ignored);Log(tr("Copied %1 entities").arg(records.size()));
}

bool xMainWnd::ExecuteOperation(QString const& operation) {
    if(!m_drawing)return false;
    auto target=m_operationsUi.opTarget->currentIndex();auto selected=SelectedIds();std::set<EntityId> targets=target==0?selected:target==1?m_workingSetIds:std::set<EntityId>(m_entityIds.begin(),m_entityIds.end());
    if(operation!="Paste" && targets.empty()){Log(tr("No target entities"));return false;}
    try{
        auto records=CloneEntityRecords(*m_drawing,m_entityIds);std::vector<EditRecord> result;std::set<EntityId> afterSelection,afterWorking=m_workingSetIds,afterHidden=m_hiddenIds;
        double x=m_operationsUi.opX->value(),y=m_operationsUi.opY->value(),z=m_operationsUi.opZ->value();
        gtl::shape::point_t pivot{x,y,z};
        if(operation=="Rotate" || operation=="Scale" || operation.startsWith("Mirror")){
            if(m_operationsUi.centerMode->currentIndex()==1)pivot={};
            else if(m_operationsUi.centerMode->currentIndex()==2){
                gtl::shape::rect_t bounds;bounds.SetRectEmptyForMinMax();bool any=false;
                for(auto const& r:records)if(targets.contains(r.id))any=r.shape->UpdateBoundary(bounds)||any;
                if(any)pivot=(bounds.pt0()+bounds.pt1())/2.;
            }
        }
        auto inherit=[&](EditRecord const& old,std::shared_ptr<Shape> shape,bool keepId){
            EntityId id=keepId?old.id:m_nextEntityId++;
            result.push_back({id,old.layer,std::move(shape)});afterSelection.insert(id);
            if(m_workingSetIds.contains(old.id))afterWorking.insert(id);
            if(m_hiddenIds.contains(old.id))afterHidden.insert(id);
        };
        if(operation=="Paste"){
            if(!m_clipboard)throw std::runtime_error("Clipboard is empty");
            size_t count=0;for(auto const& layer:m_clipboard->m_layers)count+=layer.m_shapes.size();
            size_t copies=static_cast<size_t>(m_operationsUi.pasteRows->value())*m_operationsUi.pasteColumns->value();
            if(count==0 || copies>100000/std::max<size_t>(1,count) || records.size()+copies*count>100000)throw std::runtime_error("Paste exceeds 100000 entities or clipboard is empty");
            result=std::move(records);
            for(int row=0;row<m_operationsUi.pasteRows->value();++row)for(int col=0;col<m_operationsUi.pasteColumns->value();++col)
                for(auto const& layer:m_clipboard->m_layers)for(auto const& shape:layer.m_shapes){
                    auto clone=std::shared_ptr<Shape>(shape.NewClone().release());gtl::xCoordTrans3d ct;ct.Init(1.,cv::Matx33d::eye(),{}, {x+col*m_operationsUi.pasteDX->value(),y+row*m_operationsUi.pasteDY->value(),z});TransformEntity(*clone,ct);
                    auto id=m_nextEntityId++;result.push_back({id,layer.m_name,clone});afterSelection.insert(id);
                    if(!shape.m_bVisible || !layer.m_bVisible || !m_clipboard->m_bVisible)afterHidden.insert(id);
                }
        } else if(operation=="Join"){
            std::vector<EditRecord> parts;for(auto const& record:records)if(targets.contains(record.id))parts.push_back(record);else result.push_back(record);
            if(parts.size()<2)throw std::runtime_error("Join requires at least two entities");
            double tolerance=m_operationsUi.opTolerance->value();if(!(tolerance>0))throw std::runtime_error("Join tolerance must be positive");
            auto joined=AsPolyline(*parts.front().shape);if(joined->m_bLoop || joined->m_pts.size()<2)throw std::runtime_error("Join requires open paths");
            auto first=parts.front();parts.erase(parts.begin());bool working=m_workingSetIds.contains(first.id);
            while(!parts.empty()){
                bool found=false;
                for(size_t n=0;n<parts.size();++n){
                    auto const& part=parts[n];
                    if(part.layer!=first.layer || part.shape->m_color!=first.shape->m_color || part.shape->m_lineWeight!=first.shape->m_lineWeight || part.shape->m_strLineType!=first.shape->m_strLineType || part.shape->m_eLineType!=first.shape->m_eLineType)
                        throw std::runtime_error("Join requires matching layer, color, line type and weight");
                    auto poly=AsPolyline(*part.shape);if(poly->m_bLoop || poly->m_pts.size()<2)throw std::runtime_error("Join requires open paths");
                    auto distance=[](auto const& a,auto const& b){return gtl::shape::point_t(a).Distance(gtl::shape::point_t(b));};
                    if(distance(joined->m_pts.back(),poly->m_pts.front())>tolerance && distance(joined->m_pts.back(),poly->m_pts.back())>tolerance){
                        if(distance(joined->m_pts.front(),poly->m_pts.front())>tolerance && distance(joined->m_pts.front(),poly->m_pts.back())>tolerance)continue;
                        joined->Reverse();
                    }
                    if(distance(joined->m_pts.back(),poly->m_pts.front())>tolerance)poly->Reverse();
                    joined->m_pts.back().Bulge()=poly->m_pts.front().Bulge();joined->m_pts.insert(joined->m_pts.end(),poly->m_pts.begin()+1,poly->m_pts.end());
                    working=working||m_workingSetIds.contains(part.id);parts.erase(parts.begin()+n);found=true;break;
                }
                if(!found)throw std::runtime_error("Join targets do not form a connected chain within tolerance");
            }
            if(gtl::shape::point_t(joined->m_pts.front()).Distance(gtl::shape::point_t(joined->m_pts.back()))<=tolerance){joined->m_pts.pop_back();joined->m_bLoop=true;}
            for(auto id:targets)afterWorking.erase(id);if(working)afterWorking.insert(first.id);
            result.push_back({first.id,first.layer,std::shared_ptr<Shape>(joined.release())});afterSelection.insert(first.id);
        } else {
            size_t changed=0;
            for(auto& record:records){
                if(!targets.contains(record.id)){result.push_back(record);continue;}
                auto& shape=*record.shape;
                if(operation=="Delete"){afterWorking.erase(record.id);++changed;continue;}
                if(operation=="Explode" || operation=="Break At Vertex" || operation=="Split At" || operation=="Set Start"){
                    auto* poly=dynamic_cast<gtl::shape::xPolyline*>(&shape);
                    if(!poly || poly->m_pts.size()<2){result.push_back(record);continue;}
                    auto n=poly->m_pts.size();size_t at=static_cast<size_t>(m_operationsUi.opIndex->value());
                    if(operation=="Explode"){
                        afterWorking.erase(record.id);
                        for(size_t i=0;i<(poly->m_bLoop?n:n-1);++i){
                            auto a=poly->m_pts[i],b=poly->m_pts[(i+1)%n];std::shared_ptr<Shape> piece;
                            if(a.Bulge()!=0)piece=std::make_shared<gtl::shape::xArc>(gtl::shape::xArc::GetFromBulge(a.Bulge(),a,b));
                            else {auto line=std::make_shared<gtl::shape::xLine>();line->m_pt0=a;line->m_pt1=b;piece=line;}
                            static_cast<Shape&>(*piece)=shape;inherit(record,piece,false);
                        }
                    }else if(operation=="Split At"){
                        InsertNearestVertex(*poly,pivot,m_operationsUi.opTolerance->value());inherit(record,record.shape,true);
                    }else if(operation=="Set Start"){
                        int mode=m_operationsUi.startMode->currentIndex();
                        if(mode==1){
                            at=0;double distance=std::numeric_limits<double>::infinity();
                            for(size_t i=0;i<n;++i){if(!poly->m_bLoop && i!=0 && i!=n-1)continue;double d=gtl::shape::point_t(poly->m_pts[i]).Distance(pivot);if(d<distance){distance=d;at=i;}}
                            if(!poly->m_bLoop){if(at==n-1)poly->Reverse();inherit(record,record.shape,true);++changed;continue;}
                        }else if(mode==2){at=InsertNearestVertex(*poly,pivot,m_operationsUi.opTolerance->value());n=poly->m_pts.size();}
                        if(!poly->m_bLoop || at>=n)throw std::runtime_error("Set Start requires closed polyline and valid vertex index");
                        std::rotate(poly->m_pts.begin(),poly->m_pts.begin()+at,poly->m_pts.end());inherit(record,record.shape,true);
                    }else{
                        if(at>=n)throw std::runtime_error("Split vertex index out of range");
                        afterWorking.erase(record.id);
                        if(poly->m_bLoop){std::rotate(poly->m_pts.begin(),poly->m_pts.begin()+at,poly->m_pts.end());poly->m_pts.push_back(poly->m_pts.front());poly->m_pts.back().Bulge()=0;poly->m_bLoop=false;inherit(record,record.shape,false);}
                        else{
                            if(at==0 || at+1>=n)throw std::runtime_error("Split requires an interior vertex");
                            auto second=std::make_shared<gtl::shape::xPolyline>(*poly);second->m_pts.assign(poly->m_pts.begin()+at,poly->m_pts.end());
                            poly->m_pts.resize(at+1);poly->m_pts.back().Bulge()=0;inherit(record,record.shape,false);inherit(record,second,false);
                        }
                    }++changed;continue;
                }
                if(operation=="Set Layer") {if(m_operationsUi.opText->text().trimmed().isEmpty())throw std::runtime_error("Enter a layer name");record.layer=m_operationsUi.opText->text().trimmed().toStdWString();}
                else if(operation=="Set Color"){QColor color(m_operationsUi.opText->text());if(!color.isValid())throw std::runtime_error("Enter a valid color such as #ff0000");shape.m_color=gtl::ColorRGBA(color.red(),color.green(),color.blue());}
                else if(operation=="Set Line Weight")shape.m_lineWeight=m_operationsUi.opWeight->value();
                else if(operation=="Reverse"){
                    if(auto* spline=dynamic_cast<gtl::shape::xSpline*>(&shape)){
                        std::ranges::reverse(spline->m_ptsControl);std::ranges::reverse(spline->m_ptsFit);
                        if(!spline->m_knots.empty()){double sum=spline->m_knots.front()+spline->m_knots.back();std::ranges::reverse(spline->m_knots);for(auto& knot:spline->m_knots)knot=sum-knot;}
                        std::swap(spline->m_vStart,spline->m_vEnd);spline->m_vStart=-spline->m_vStart;spline->m_vEnd=-spline->m_vEnd;
                    }else shape.Reverse();
                }
                else if(operation=="Translate" || operation=="Rotate" || operation=="Scale" || operation.startsWith("Mirror")){
                    gtl::xCoordTrans3d ct;ct.Init();
                    if(operation=="Translate")ct.m_offset=pivot;
                    else if(operation=="Rotate"){
                        double angle=m_operationsUi.opAngle->value()*std::numbers::pi/180.;double c=std::cos(angle),s=std::sin(angle);
                        ct.Init(1.,cv::Matx33d{c,-s,0,s,c,0,0,0,1},pivot,pivot);
                    }else if(operation=="Scale"){
                        if(!(m_operationsUi.opScale->value()>0))throw std::runtime_error("Scale must be positive (use Mirror for reflection)");ct.Init(m_operationsUi.opScale->value(),cv::Matx33d::eye(),pivot,pivot);
                    }else{
                        ct.Init(1.,cv::Matx33d::eye(),pivot,{});TransformEntity(shape,ct);
                        MirrorEntity(shape,operation=="Mirror X"?0:operation=="Mirror Y"?1:2);
                        ct.Init(1.,cv::Matx33d::eye(),{},pivot);
                    }
                    TransformEntity(shape,ct);
                }else throw std::runtime_error("Unknown operation");
                inherit(record,record.shape,true);++changed;
            }
            if(changed==0){Log(tr("No supported targets for %1").arg(operation));return false;}
        }
        if(result.size()>100000)throw std::runtime_error("Edit exceeds 100000 entities");
        auto source=*m_drawing;
        if(operation=="Paste" && m_clipboard){
            // Bring line-type definitions with pasted/imported entities. Rename conflicts.
            for(auto const& incoming:m_clipboard->m_line_types){
                auto found=std::find_if(source.m_line_types.begin(),source.m_line_types.end(),[&](auto const& v){return v.name==incoming.name;});
                auto copy=incoming;
                if(found!=source.m_line_types.end()){
                    if(*found==incoming)continue;
                    int suffix=1;do{copy.name=incoming.name+L"_import"+std::to_wstring(suffix++);}while(std::any_of(source.m_line_types.begin(),source.m_line_types.end(),[&](auto const& v){return v.name==copy.name;}));
                    for(auto& r:result)if(afterSelection.contains(r.id) && r.shape->m_strLineType==incoming.name)r.shape->m_strLineType=copy.name;
                }
                source.m_line_types.push_back(new gtl::shape::line_type_t(copy));
            }
        }
        std::vector<EntityId> ids;auto drawing=BuildDrawing(source,result,ids);
        if(!m_history.empty())m_history[m_historyPosition]=CaptureState(m_history[m_historyPosition].label);
        m_drawing=std::move(drawing);m_entityIds=std::move(ids);m_workingSetIds=std::move(afterWorking);m_revision=m_nextRevision++;
        std::set<EntityId> live(m_entityIds.begin(),m_entityIds.end());std::erase_if(afterHidden,[&](auto id){return !live.contains(id);});m_hiddenIds=std::move(afterHidden);
        RefreshDrawing(afterSelection);PushHistory(operation);Log(tr("%1: %2 result entities selected").arg(operation).arg(afterSelection.size()));return true;
    }catch(std::exception const& e){Log(tr("Edit rejected: %1").arg(QString::fromUtf8(e.what())));return false;}
}

void xMainWnd::SetupEditor() {
    // Keep document shortcuts available while either modeless tool window is active.
    for(auto* dialog:{m_operationsDialog,m_filterDialog})
        for(auto* action:{m_ui.actionOpen,m_ui.actionSave,m_ui.actionSaveAs,m_ui.actionUndo,m_ui.actionRedo,m_ui.actionCopy,m_ui.actionPaste,m_ui.actionDelete})dialog->addAction(action);
    for(auto pair:{std::pair{m_operationsDialog,m_ui.actionOperations},std::pair{m_filterDialog,m_ui.actionSelectionFilter}}){
        connect(pair.second,&QAction::toggled,this,[dialog=pair.first](bool visible){dialog->setVisible(visible);if(visible){dialog->raise();dialog->activateWindow();}});
    }
    for(auto pair:{std::pair{m_ui.entityDock,m_ui.actionEntities},std::pair{m_ui.propertyDock,m_ui.actionProperties},
        std::pair{m_ui.workingDock,m_ui.actionWorkingSet},std::pair{m_ui.historyDock,m_ui.actionHistory},std::pair{m_ui.logDock,m_ui.actionLog}}){
        connect(pair.first,&QDockWidget::visibilityChanged,this,[dock=pair.first,action=pair.second](bool){QSignalBlocker block(action);action->setChecked(!dock->isHidden());});
    }
    auto parameters=[this]{
        auto op=m_operationsUi.operation->currentText();bool coordinates=op=="Translate" || op=="Rotate" || op=="Scale" || op.startsWith("Mirror") || op=="Paste" || op=="Split At" || (op=="Set Start" && m_operationsUi.startMode->currentIndex()!=0);
        m_operationsUi.operationParameters->setRowVisible(m_operationsUi.opX,coordinates);m_operationsUi.operationParameters->setRowVisible(m_operationsUi.opY,coordinates);m_operationsUi.operationParameters->setRowVisible(m_operationsUi.opZ,coordinates);
        m_operationsUi.operationParameters->setRowVisible(m_operationsUi.startMode,op=="Set Start");
        m_operationsUi.operationParameters->setRowVisible(m_operationsUi.centerMode,op=="Rotate" || op=="Scale" || op.startsWith("Mirror"));
        m_operationsUi.operationParameters->setRowVisible(m_operationsUi.opAngle,op=="Rotate");m_operationsUi.operationParameters->setRowVisible(m_operationsUi.opScale,op=="Scale");
        m_operationsUi.operationParameters->setRowVisible(m_operationsUi.opTolerance,op=="Join" || op=="Split At" || (op=="Set Start" && m_operationsUi.startMode->currentIndex()==2));m_operationsUi.operationParameters->setRowVisible(m_operationsUi.opIndex,(op=="Set Start" && m_operationsUi.startMode->currentIndex()==0) || op=="Break At Vertex");
        m_operationsUi.operationParameters->setRowVisible(m_operationsUi.opWeight,op=="Set Line Weight");m_operationsUi.operationParameters->setRowVisible(m_operationsUi.opText,op=="Set Color" || op=="Set Layer");
        for(auto* field:{m_operationsUi.pasteRows,m_operationsUi.pasteColumns})m_operationsUi.operationParameters->setRowVisible(field,op=="Paste");
        for(auto* field:{m_operationsUi.pasteDX,m_operationsUi.pasteDY})m_operationsUi.operationParameters->setRowVisible(field,op=="Paste");m_operationsUi.pickPoint->setVisible(coordinates);
    };
    connect(m_operationsUi.operation,&QComboBox::currentTextChanged,this,parameters);connect(m_operationsUi.startMode,&QComboBox::currentIndexChanged,this,parameters);parameters();
    auto save=[this](bool as){
        if(!m_drawing)return;auto path=as?QString{}:m_filePath;
        if(path.isEmpty() || !path.endsWith(".shape",Qt::CaseInsensitive))path=QFileDialog::getSaveFileName(this,tr("Save drawing"),QFileInfo(m_filePath).completeBaseName()+".shape",tr("Shape (*.shape)"));
        if(path.isEmpty())return;if(!path.endsWith(".shape",Qt::CaseInsensitive))path+=".shape";SaveShape(path);
    };
    connect(m_ui.actionSave,&QAction::triggered,this,[save]{save(false);});connect(m_ui.actionSaveAs,&QAction::triggered,this,[save]{save(true);});
    auto exportIds=[this](std::set<EntityId> ids){if(ids.empty())return;auto path=QFileDialog::getSaveFileName(this,tr("Export entities"),"selection.shape",tr("Shape (*.shape)"));if(path.isEmpty())return;if(!path.endsWith(".shape",Qt::CaseInsensitive))path+=".shape";ExportShapes(path,ids);};
    connect(m_ui.actionExportSelection,&QAction::triggered,this,[this,exportIds]{exportIds(SelectedIds());});
    connect(m_ui.workingExport,&QPushButton::clicked,this,[this,exportIds]{exportIds(m_workingSetIds);});
    connect(m_ui.actionSavePng,&QAction::triggered,this,[this]{auto path=QFileDialog::getSaveFileName(this,tr("Save view"),"drawing.png",tr("PNG (*.png)"));if(!path.isEmpty()){if(!path.endsWith(".png",Qt::CaseInsensitive))path+=".png";Log(m_ui.drawingView->grab().save(path)?tr("PNG saved"):tr("PNG save failed"));}});
    connect(m_ui.actionCopy,&QAction::triggered,this,&xMainWnd::CopySelection);
    connect(m_ui.actionPaste,&QAction::triggered,this,[this]{if(IsEditingText())return;m_operationsUi.operation->setCurrentText("Paste");m_ui.actionOperations->setChecked(true);m_operationsDialog->raise();m_operationsDialog->activateWindow();});
    connect(m_ui.actionDelete,&QAction::triggered,this,[this]{if(IsEditingText())return;auto target=m_operationsUi.opTarget->currentIndex();m_operationsUi.opTarget->setCurrentIndex(0);ExecuteOperation("Delete");m_operationsUi.opTarget->setCurrentIndex(target);});
    connect(m_operationsUi.applyOperation,&QPushButton::clicked,this,[this]{ExecuteOperation(m_operationsUi.operation->currentText());});
    connect(m_operationsUi.pickPoint,&QPushButton::clicked,this,[this]{m_isPickingPoint=true;Log(tr("Click the drawing to set X/Y; Escape cancels"));});
    connect(m_ui.actionUndo,&QAction::triggered,this,[this]{if(IsEditingText())return;if(m_historyPosition>0)RestoreHistory(m_historyPosition-1);});
    connect(m_ui.actionRedo,&QAction::triggered,this,[this]{if(!IsEditingText())RestoreHistory(m_historyPosition+1);});
    connect(m_ui.historyList,&QListWidget::currentRowChanged,this,[this](int row){if(row>=0)RestoreHistory(static_cast<size_t>(row));});
    connect(m_ui.historyClear,&QPushButton::clicked,this,[this]{if(!m_drawing)return;m_history.clear();PushHistory(tr("Current state"));});
    connect(m_ui.workingAdd,&QPushButton::clicked,this,[this]{auto ids=SelectedIds();m_workingSetIds.insert(ids.begin(),ids.end());RefreshEditorPanels();});
    connect(m_ui.workingRemove,&QPushButton::clicked,this,[this]{for(auto* item:m_ui.workingList->selectedItems())m_workingSetIds.erase(item->data(Qt::UserRole).toULongLong());RefreshEditorPanels();});
    connect(m_ui.workingClear,&QPushButton::clicked,this,[this]{m_workingSetIds.clear();RefreshEditorPanels();});
    connect(m_ui.workingSelect,&QPushButton::clicked,this,[this]{SelectIds(m_workingSetIds);});
    connect(m_ui.workingList,&QListWidget::itemDoubleClicked,this,[this](QListWidgetItem* item){SelectIds({item->data(Qt::UserRole).toULongLong()});FitTreeItem(m_ui.entityTree->currentItem());});
    connect(m_filterUi.selectMatches,&QPushButton::clicked,this,[this]{std::set<EntityId> ids;for(size_t i=0;i<m_entityIds.size();++i)if(m_displayShapes[i]->m_bVisible && MatchesFilter(i))ids.insert(m_entityIds[i]);SelectIds(ids);});
    connect(m_filterUi.selectAll,&QPushButton::clicked,this,[this]{std::set<EntityId> ids;for(size_t i=0;i<m_entityIds.size();++i)if(m_displayShapes[i]->m_bVisible)ids.insert(m_entityIds[i]);SelectIds(ids);});
    connect(m_filterUi.invertSelection,&QPushButton::clicked,this,[this]{auto selected=SelectedIds();std::set<EntityId> ids;for(size_t i=0;i<m_entityIds.size();++i)if(m_displayShapes[i]->m_bVisible && !selected.contains(m_entityIds[i]))ids.insert(m_entityIds[i]);SelectIds(ids);});
    connect(m_ui.actionDark,&QAction::toggled,this,[this](bool dark){m_ui.drawingView->SetBackgroundColor(dark?Qt::black:Qt::white);RefreshDrawing(SelectedIds());});
    connect(m_ui.actionShowText,&QAction::toggled,this,[this]{RefreshDrawing(SelectedIds());});
    connect(m_ui.actionDirections,&QAction::toggled,this,[this]{RefreshSelectionPresentation();});
    auto layout=[this]{
        m_ui.entityDock->setFloating(false);m_ui.propertyDock->setFloating(false);
        addDockWidget(Qt::LeftDockWidgetArea,m_ui.entityDock);addDockWidget(Qt::RightDockWidgetArea,m_ui.propertyDock);
        for(auto* dock:{m_ui.workingDock,m_ui.historyDock,m_ui.logDock}){dock->setFloating(false);addDockWidget(Qt::BottomDockWidgetArea,dock);dock->show();}
        tabifyDockWidget(m_ui.workingDock,m_ui.historyDock);tabifyDockWidget(m_ui.workingDock,m_ui.logDock);
        m_ui.workingDock->raise();resizeDocks({m_ui.workingDock},{250},Qt::Vertical);m_ui.entityDock->show();m_ui.propertyDock->show();
    };
    layout();connect(m_ui.actionResetLayout,&QAction::triggered,this,layout);
    if(auto state=theApp->GetReg().value("CADViewer/layout").toByteArray();!state.isEmpty())restoreState(state);
    connect(m_ui.actionImportShape,&QAction::triggered,this,[this]{
        auto path=QFileDialog::getOpenFileName(this,tr("Import entities"),{},tr("Shape (*.shape)"));if(path.isEmpty())return;
        try{auto imported=ReadShapeFile(path);if(!m_drawing){OpenFile(path);return;}auto clipboard=m_clipboard;m_clipboard=imported;
            // Import at source coordinates, independent of current paste controls.
            auto x=m_operationsUi.opX->value(),y=m_operationsUi.opY->value(),z=m_operationsUi.opZ->value();int rows=m_operationsUi.pasteRows->value(),cols=m_operationsUi.pasteColumns->value();
            m_operationsUi.opX->setValue(0);m_operationsUi.opY->setValue(0);m_operationsUi.opZ->setValue(0);m_operationsUi.pasteRows->setValue(1);m_operationsUi.pasteColumns->setValue(1);ExecuteOperation("Paste");
            m_operationsUi.opX->setValue(x);m_operationsUi.opY->setValue(y);m_operationsUi.opZ->setValue(z);m_operationsUi.pasteRows->setValue(rows);m_operationsUi.pasteColumns->setValue(cols);m_clipboard=clipboard;
        }catch(std::exception const& e){Log(QString::fromUtf8(e.what()));}
    });
    RefreshEditorPanels();
}

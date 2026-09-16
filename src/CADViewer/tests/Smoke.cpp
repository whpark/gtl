#include "pch.h"
#include <fstream>
#define main CADViewerProductionMain
#include "../App.cpp"
#undef main
#include "../MainWnd.cpp"
#include "gtl/qt/MatView/LayeredMatView.h"
int main(int argc, char** argv) { try {
 QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
 QSettings::setDefaultFormat(QSettings::IniFormat);
 QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,"build-dwg/verification/viewer-smoke/settings");
 theApp.emplace(argc,argv);theApp->Init();auto& wnd=theApp->GetMainWnd();
 auto* tree=wnd.findChild<QTreeWidget*>("entityTree");auto* props=wnd.findChild<QTreeWidget*>("propertyTree");auto* view=wnd.findChild<gtl::qt::xLayeredMatView*>("drawingView");
 auto check=[](bool ok,char const* message){if(!ok){std::ofstream("build-dwg/verification/viewer-smoke/failure.txt")<<message;std::exit(2);}};
 {
  gtl::shape::xRay ray;ray.m_origin={-20,5,0};ray.m_direction={1,0,0};
  xSelectionCanvas hit(QRectF(0,0,10,10),1.);ray.Draw(hit);check(hit.any && hit.crossing,"ray crossing selection missing");
  ray.m_origin.y=20;xSelectionCanvas miss(QRectF(0,0,10,10),1.);ray.Draw(miss);check(!miss.any,"ray outside region selected");
 }
 check(wnd.findChild<QAction*>("actionDirections")->isChecked(),"endpoint marks not enabled by default");
 for(auto path:{"build-dwg/verification/viewer-smoke/sample.dxf","src/test_dwg/DWG/sample_AC1032.dwg"}) {
  check(wnd.OpenFile(path),"open failed");QApplication::processEvents();
  check(view->Count()==1,"base drawing count");QTreeWidgetItem* selected{};
  for(int i=0;i<tree->topLevelItemCount();++i)if(tree->topLevelItem(i)->childCount()){selected=tree->topLevelItem(i)->child(0);break;}
  check(selected,"missing entities");tree->expandItem(selected->parent());tree->setCurrentItem(selected);QApplication::processEvents();
  check(props->topLevelItemCount()>=6,"missing properties");check(view->Count()==2,"highlight missing");
  check(!view->GetSelectionPoints().has_value(),"selected entity retained filled selection rectangle");
  auto marked=view->grab().toImage();wnd.findChild<QAction*>("actionDirections")->setChecked(false);QApplication::processEvents();
  if(QString(path).endsWith("dxf"))check(marked!=view->grab().toImage(),"line endpoint arrows toggle has no visual effect");
  check(tree->selectedItems().contains(selected) && view->Count()==2,"endpoint toggle changed selection highlight");
  wnd.findChild<QAction*>("actionDirections")->setChecked(true);
  auto* layer=selected->parent();
  layer->setCheckState(0,Qt::Unchecked);QApplication::processEvents();
  for(int i=0;i<layer->childCount();++i)check(layer->child(i)->checkState(0)==Qt::Unchecked,"layer did not hide child");
  check(view->Count()==1,"layer hide retained selection overlay");
  layer->setCheckState(0,Qt::Checked);QApplication::processEvents();
  for(int i=0;i<layer->childCount();++i)check(layer->child(i)->checkState(0)==Qt::Checked,"layer did not show child");
  check(view->Count()==2,"layer show did not restore selection overlay");
  if(layer->childCount()>1){
   layer->child(1)->setCheckState(0,Qt::Unchecked);
   check(layer->checkState(0)==Qt::PartiallyChecked,"layer missing partial state");
   layer->child(1)->setCheckState(0,Qt::Checked);
   check(layer->checkState(0)==Qt::Checked,"layer missing checked state");
  }
  auto shown=view->grab().toImage();
  selected->setCheckState(0,Qt::Unchecked);QApplication::processEvents();
  check(view->Count()==1,"hidden entity retained highlight");
  check(!view->GetSelectionPoints().has_value(),"hidden entity retained selection bounds");
  check(props->topLevelItemCount()>=6,"hidden entity lost properties");
  check(shown!=view->grab().toImage(),"visibility toggle did not change rendered image");
  tree->setCurrentItem(nullptr);tree->setCurrentItem(selected);QApplication::processEvents();
  check(view->Count()==1,"selecting hidden entity restored highlight");
  selected->setCheckState(0,Qt::Checked);QApplication::processEvents();
  check(view->Count()==2,"checking selected entity did not restore highlight");
  check(!view->GetSelectionPoints().has_value(),"checking selected entity restored selection rectangle");

  wnd.grab().save(QString("build-dwg/verification/viewer-smoke/")+ (QString(path).endsWith("dxf")?"dxf.png":"dwg.png"));
  tree->setCurrentItem(nullptr);QApplication::processEvents();check(view->Count()==1,"highlight not cleared");check(props->topLevelItemCount()==0,"properties not cleared");
 }

 check(wnd.OpenFile("build-dwg/verification/viewer-smoke/sample.dxf"),"selection fixture open");QApplication::processEvents();
 auto* fitLayer=tree->topLevelItem(0);auto* fitCircle=fitLayer->child(1);
 tree->setCurrentItem(fitCircle);tree->itemDoubleClicked(fitCircle,0);QApplication::processEvents();
 auto viewport=view->rect();if(auto* toolbar=view->findChild<QWidget*>("toolbar");toolbar&&toolbar->isVisible())viewport.setTop(toolbar->geometry().bottom()+1);
 auto center=view->MapFromWorld({50,-30});
 check(std::abs(center.x-(viewport.left()+viewport.width()/2.))<.01 && std::abs(center.y-(viewport.top()+viewport.height()/2.))<.01,"double click circle not centered");
 check(40*view->Zoom()<=viewport.width()-39 && 40*view->Zoom()<=viewport.height()-39,"circle fit clipped");
 fitCircle->setCheckState(0,Qt::Unchecked);auto oldPan=view->Pan();auto oldZoom=view->Zoom();
 tree->itemDoubleClicked(fitCircle,0);check(view->Pan()==oldPan && view->Zoom()==oldZoom,"hidden fit changed camera");
 fitCircle->setCheckState(0,Qt::Checked);tree->itemDoubleClicked(fitLayer,0);QApplication::processEvents();
 center=view->MapFromWorld({50,-25});
 check(std::abs(center.x-(viewport.left()+viewport.width()/2.))<.01 && std::abs(center.y-(viewport.top()+viewport.height()/2.))<.01,"double click layer not centered");
 check(100*view->Zoom()<=viewport.width()-39 && 50*view->Zoom()<=viewport.height()-39,"layer fit clipped");
 auto drag=[&](double x0,double y0,double x1,double y1){
  auto a=view->MapFromWorld({x0,-y0}),b=view->MapFromWorld({x1,-y1});
  QPointF p0(a.x,a.y),p1(b.x,b.y);
  QMouseEvent press(QEvent::MouseButtonPress,p0,p0,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
  QMouseEvent move(QEvent::MouseMove,p1,p1,Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
  QMouseEvent release(QEvent::MouseButtonRelease,p1,p1,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
  QApplication::sendEvent(view,&press);QApplication::sendEvent(view,&move);QApplication::sendEvent(view,&release);QApplication::processEvents();
 };
 drag(-5,-5,105,55);check(tree->selectedItems().size()==2,"window did not select both contained entities");
 check(!view->GetSelectionPoints().has_value(),"multi selection retained filled rectangle");
 check(props->topLevelItemCount()==2,"multi properties missing");check(view->Count()==2,"multi overlay missing");
 wnd.grab().save("build-dwg/verification/viewer-smoke/multi.png");
 drag(45,-5,55,55);check(tree->selectedItems().isEmpty(),"window selected partial entities");
 drag(55,-5,45,55);check(tree->selectedItems().size()==2,"crossing did not select intersected entities");
 drag(51,29,49,31);check(tree->selectedItems().isEmpty(),"circle bounding-box false hit");
 drag(15,40,10,45);check(tree->selectedItems().isEmpty(),"line bounding-box false hit");
 auto* parent=tree->topLevelItem(0);parent->setCheckState(0,Qt::Unchecked);
 drag(-5,-5,105,55);check(tree->selectedItems().isEmpty(),"hidden entities selected");
 parent->setCheckState(0,Qt::Checked);
 drag(-5,-5,105,55);check(tree->selectedItems().size()==2,"reshown entities not selected");
 tree->clearSelection();QApplication::processEvents();check(view->Count()==1,"multi clear overlay");
 QTimer::singleShot(0,[]{for(auto* w:QApplication::topLevelWidgets())if(auto* box=qobject_cast<QMessageBox*>(w))box->accept();});
 check(!wnd.OpenFile("missing.dwg"),"missing file accepted");check(view->Count()==1,"failed open removed drawing");
 {
  using namespace gtl::shape;
  xDrawing drawing;auto layer=std::make_unique<xLayer>(L"CAD");
  auto line=std::make_unique<xLine>();line->m_pt0={0,0,0};line->m_pt1={10,0,0};layer->m_shapes.push_back(std::move(line));
  auto ray=std::make_unique<xRay>();ray->m_origin={0,5,0};ray->m_direction={1,0,0};layer->m_shapes.push_back(std::move(ray));
  auto cad=std::make_unique<x3DSolid>();cad->m_entityName=L"3DSOLID";cad->m_groups={{1,string_t{L"body"}}};cad->m_binary.emplace();cad->m_binary->bytes={0,128,255};cad->m_origin={2,3,4};layer->m_shapes.push_back(std::move(cad));drawing.m_layers.push_back(std::move(layer));
  auto path=QString("build-dwg/verification/viewer-smoke/cad.shape");WriteShapeFile(path,drawing);check(wnd.OpenFile(path),"CAD shape open failed");QApplication::processEvents();
  drag(2,4,8,6);check(tree->selectedItems().isEmpty(),"window selected unbounded ray");
  drag(8,4,2,6);check(tree->selectedItems().size()==1,"crossing did not select ray");check(view->Count()==2,"selected ray highlight missing");
  check(wnd.SaveShape(path),"CAD shape save failed");auto restored=ReadShapeFile(path);
  auto data=dynamic_cast<x3DSolid const*>(&restored->m_layers.front().m_shapes[2]);check(data && data->m_binary && data->m_binary->bytes==std::vector<uint8_t>{0,128,255} && data->m_origin==point_t{2,3,4},"CAD payload or placement lost in .shape");
 }
 {
  auto legacy=ReadShapeFile("src/CADViewer/tests/legacy-cad-v0.shape");
  auto* cad=dynamic_cast<gtl::shape::xCadEntity const*>(&legacy->m_layers.front().m_shapes[2]);
  check(cad && !cad->m_bExternalGraphics && cad->m_binary && cad->m_binary->bytes==std::vector<uint8_t>{0,128,255},"legacy CAD .shape archive failed");
 }
 {
  using namespace gtl::shape;
  xDrawing drawing;auto layer=std::make_unique<xLayer>(L"Annotations");auto leader=std::make_unique<xLeader>();
  leader->m_groups={{10,0.},{20,0.},{10,10.},{20,10.},{10,20.},{20,10.},{71,int16_t{1}}};layer->m_shapes.push_back(std::move(leader));drawing.m_layers.push_back(std::move(layer));
  auto path=QString("build-dwg/verification/viewer-smoke/leader.shape");WriteShapeFile(path,drawing);check(wnd.OpenFile(path),"leader open failed");QApplication::processEvents();
  drag(-5,-5,25,15);check(tree->selectedItems().size()==1 && view->Count()==2,"CAD leader selection or highlight missing");
  auto image=view->grab().toImage();tree->topLevelItem(0)->setCheckState(0,Qt::Unchecked);QApplication::processEvents();check(image!=view->grab().toImage(),"CAD leader was not rendered");
  wnd.grab().save("build-dwg/verification/viewer-smoke/cad-leader.png");
 }
 #include "EditorChecks.inc"
 qInfo()<<"CADViewer smoke passed: DXF, DWG, entity selection, properties, highlight, clear and failed load.";
 theApp.reset();return 0;
} catch(std::exception const& e){std::ofstream("build-dwg/verification/viewer-smoke/failure.txt")<<e.what();return 3;}
}

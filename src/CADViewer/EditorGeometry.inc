// CADViewer editing uses uniform scale, XY rotation and translation.
// Correct legacy Shape transform omissions without changing the shared archive/API.
namespace {
void TransformEntity(Shape& shape,gtl::xCoordTrans3d const& ct) {
    using namespace gtl::shape;
    auto zero=ct(point_t{}),axis=ct(point_t{1,0,0})-zero;
    double scale=std::hypot(axis.x,axis.y);gtl::deg_t angle{std::atan2(axis.y,axis.x)*180./std::numbers::pi};
    if(auto* layer=dynamic_cast<xLayer*>(&shape)){for(auto& child:layer->m_shapes)TransformEntity(child,ct);}
    else if(auto* hatch=dynamic_cast<xHatch*>(&shape)){for(auto& boundary:hatch->m_boundaries)TransformEntity(boundary,ct);hatch->m_angle+=angle;hatch->m_scale*=scale;}
    else if(auto* spline=dynamic_cast<xSpline*>(&shape)){
        spline->m_ptNormal=(ct(spline->m_ptNormal)-zero)/std::max(scale,1e-12);spline->m_vStart=ct(spline->m_vStart)-zero;spline->m_vEnd=ct(spline->m_vEnd)-zero;
        for(auto& p:spline->m_ptsControl)p=ct(p);for(auto& p:spline->m_ptsFit)p=ct(p);
    }else {
        shape.Transform(ct,true);
        if(auto* ellipse=dynamic_cast<xEllipse*>(&shape))ellipse->m_angle_first_axis+=angle;
        else if(auto* arc=dynamic_cast<xArc*>(&shape))arc->m_angle_start+=angle;
        if(auto* text=dynamic_cast<xText*>(&shape)){text->m_angle+=angle;text->m_height*=scale;}
    }
}
void MirrorEntity(Shape& shape,int axis) {
    using namespace gtl::shape;
    if(auto* layer=dynamic_cast<xLayer*>(&shape)){for(auto& child:layer->m_shapes)MirrorEntity(child,axis);return;}
    if(auto* hatch=dynamic_cast<xHatch*>(&shape)){for(auto& b:hatch->m_boundaries)MirrorEntity(b,axis);return;}
    if(auto* ellipse=dynamic_cast<xEllipse*>(&shape)){
        if(axis==2){ellipse->m_ptCenter.z=-ellipse->m_ptCenter.z;return;}
        if(axis==0)ellipse->m_ptCenter.x=-ellipse->m_ptCenter.x;else ellipse->m_ptCenter.y=-ellipse->m_ptCenter.y;
        ellipse->m_angle_first_axis=gtl::deg_t{axis==0?180.:0.}-ellipse->m_angle_first_axis;
        ellipse->m_angle_start=-ellipse->m_angle_start;ellipse->m_angle_length=-ellipse->m_angle_length;return;
    }
    if(axis==2){
        if(auto* poly=dynamic_cast<xPolyline*>(&shape)){for(auto& p:poly->m_pts)p.z=-p.z;return;}
        if(auto* circle=dynamic_cast<xCircle*>(&shape)){circle->m_ptCenter.z=-circle->m_ptCenter.z;return;}
    }
    if(axis==0)shape.FlipX();else if(axis==1)shape.FlipY();else shape.FlipZ();
    if(axis!=2)if(auto* text=dynamic_cast<xText*>(&shape)){text->m_angle=gtl::deg_t{axis==0?180.:0.}-text->m_angle;text->m_textgen^=4;}
}
// Insert the closest point on the polyline, preserving arc bulges and its outline.
size_t InsertNearestVertex(gtl::shape::xPolyline& poly,gtl::shape::point_t const& point,double tolerance) {
    using namespace gtl::shape;
    if(tolerance<0)throw std::runtime_error("Tolerance must be non-negative");
    size_t bestSegment=0;double bestT=0,bestDistance=std::numeric_limits<double>::infinity();point_t bestPoint;
    size_t n=poly.m_pts.size();
    for(size_t i=0;i<(poly.m_bLoop?n:n-1);++i){
        point_t a=poly.m_pts[i],b=poly.m_pts[(i+1)%n];double bulge=poly.m_pts[i].Bulge();
        auto consider=[&](double t,point_t q){double d=q.Distance(point);if(d<bestDistance){bestDistance=d;bestSegment=i;bestT=t;bestPoint=q;}};
        consider(0,a);consider(1,b);
        if(std::abs(bulge)<1e-14){
            auto delta=b-a;double length2=delta.x*delta.x+delta.y*delta.y+delta.z*delta.z;
            if(length2>0){auto offset=point-a;double t=std::clamp((offset.x*delta.x+offset.y*delta.y+offset.z*delta.z)/length2,0.,1.);consider(t,a+delta*t);}
        }else{
            auto arc=xArc::GetFromBulge(bulge,a,b);double start=static_cast<double>(gtl::rad_t(arc.m_angle_start)),sweep=4*std::atan(bulge);
            double angle=std::atan2(point.y-arc.m_ptCenter.y,point.x-arc.m_ptCenter.x),turn=2*std::numbers::pi;
            double travel=std::fmod(sweep>0?angle-start:start-angle,turn);if(travel<0)travel+=turn;
            double t=travel/std::abs(sweep);if(t>0 && t<1){double at=start+t*sweep;consider(t,{arc.m_ptCenter.x+arc.m_radius*std::cos(at),arc.m_ptCenter.y+arc.m_radius*std::sin(at),a.z+t*(b.z-a.z)});}
        }
    }
    auto next=(bestSegment+1)%n;
    if(bestT<=0 || bestPoint.Distance(point_t(poly.m_pts[bestSegment]))<=tolerance)return bestSegment;
    if(bestT>=1 || bestPoint.Distance(point_t(poly.m_pts[next]))<=tolerance)return next;
    double sweep=4*std::atan(poly.m_pts[bestSegment].Bulge());
    polypoint_t inserted(bestPoint);inserted.Bulge()=std::tan(sweep*(1-bestT)/4);
    poly.m_pts[bestSegment].Bulge()=std::tan(sweep*bestT/4);
    poly.m_pts.insert(poly.m_pts.begin()+bestSegment+1,inserted);return bestSegment+1;
}
}

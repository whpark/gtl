#include "pch.h"
#include "gtl/shape/shape.h"

BOOST_CLASS_EXPORT_GUID(gtl::shape::s_layer, "layer")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_dot, "dot")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_line, "line")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_polyline, "polyline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_circleXY, "circleXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_arcXY, "arcXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_ellipseXY, "ellipseXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_spline, "spline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_text, "text")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_mtext, "mtext")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_hatch, "hatch")

namespace gtl::shape {

	void s_polyline::Draw(ICanvas& canvas) const {
		for (int iPt = 0; iPt < pts.size(); iPt++) {
			polypoint_t pt0{pts[iPt]};
			polypoint_t pt1;
			bool bLast = false;
			if (bLoop) {
				if (iPt == pts.size()-1) {
					pt1 = pts[0];
					bLast = true;
				} else {
					pt1 = pts[iPt+1];
				}
			} else {
				if (iPt == pts.size()-1)
					break;
				bLast = iPt == pts.size()-2;
				pt1 = pts[iPt+1];
			}

			double dBulge = pt0.Bulge();

			if (dBulge == 0.0) {
				canvas.LineTo(pt1);
			} else {
				canvas.LineTo(pt0);
				s_arcXY arc = s_arcXY::GetFromBulge(dBulge, pt0, pt1);
				(s_shape&)arc = *(s_shape*)this;

				arc.Draw(canvas);

				canvas.LineTo(pt1);
			}
		}
	}

	bool s_drawing::FromJson(boost::json::value& json) {

		//json.at();


		return true;
	}

};

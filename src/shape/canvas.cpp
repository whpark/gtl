#include "pch.h"
#include "gtl/shape/canvas.h"

#include "../3rdparty/tinyspline/tinysplinecxx.h"

namespace gtl::shape {

	GTL__SHAPE_API void Canvas_Spline(ICanvas& canvas, int degree, std::span<point_t const> pts, std::span<double const> knots, bool bLoop) {
		if (pts.size() < 2)
			return;

		double len{};
		for (size_t i{1}; i < pts.size(); i++) {
			len += pts[i].Distance(pts[i-1]);
		}

		double scale = canvas.m_ct.Trans(1.0);
		double step = canvas.m_target_interpolation_inverval / len / scale;
		//double step = 0.001;

		constexpr static int const dim = 2;
		tinyspline::BSpline bspline(pts.size(), dim, degree);
		std::vector<tinyspline::real> points((pts.size()+bLoop)*dim);
		auto* pos = points.data();
		if constexpr (dim == 2) {
			for (size_t i{}; i < pts.size(); i++) {
				*pos++ += pts[i].x;
				*pos++ += pts[i].y;
			}
			//if (bLoop) {
			//	*pos++ += pts[0].x;
			//	*pos++ += pts[0].y;
			//}
		} else if (dim == 3) {
			for (size_t i{}; i < pts.size(); i++) {
				*pos++ += pts[i].x;
				*pos++ += pts[i].y;
				*pos++ += pts[i].z;
			}
			//if (bLoop) {
			//	*pos++ += pts[0].x;
			//	*pos++ += pts[0].y;
			//	*pos++ += pts[0].z;
			//}
		} else {
			// todo : error.
			return;
		}
		bspline.setControlPoints(points);
		//std::vector<double> knotsv{knots.begin(), knots.end()};
		//bspline.setKnots(knotsv);

		//auto cr = dynamic_cast<xCanvasMat&>(canvas).m_color;
		//dynamic_cast<xCanvasMat&>(canvas).m_color = cv::Scalar(0, 0, 255);
		//canvas.MoveTo(pts.front());
		//for (auto const& pt : pts) {
		//	canvas.LineTo(pt);
		//}
		//dynamic_cast<xCanvasMat&>(canvas).m_color = cr;

		point_t pt{bspline.eval(0.0).result()};
		canvas.MoveTo(pt);
		for (double t{ step }; t < 1.0; t += step) {
			auto pt = bspline.eval(t).result();
			canvas.LineTo(point_t(pt));
		}
		canvas.LineTo(point_t(bspline.eval(1.).result()));

	}

}

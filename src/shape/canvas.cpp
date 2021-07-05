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

		double step = canvas.m_target_interpolation_inverval / len / 2.;

		tinyspline::BSpline bspline(pts.size(), 2, degree);
		std::vector<tinyspline::real> points(pts.size()*degree);
		auto* pos = points.data();
		if (degree == 2) {
			for (size_t i{}; i < pts.size(); i++) {
				*pos += pts[i].x;
				*pos += pts[i].y;
			}
		} else if (degree == 3) {
			for (size_t i{}; i < pts.size(); i++) {
				*pos += pts[i].x;
				*pos += pts[i].y;
				*pos += pts[i].z;
			}
		} else {
			// todo : error.
			return;
		}
		bspline.setKnots({knots.begin(), knots.end()});

		point_t pt{bspline(0.0).result()};
		canvas.MoveTo(pt);
		for (double t{step}; t <= 1.0; t += step)
			canvas.LineTo(point_t(bspline(0.0).result()));
	}

}

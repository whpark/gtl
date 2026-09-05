#pragma once

#include "gtl/dwg/aliases.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace gtl::dwg::detail {
	inline point_t Add(point_t a, point_t b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
	inline point_t Scale(point_t a, double s) { return {a.x*s, a.y*s, a.z*s}; }
	inline point_t Cross(point_t a, point_t b) { return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }
	inline double Dot(point_t a, point_t b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
	inline double Length(point_t a) { return std::hypot(a.x, a.y, a.z); }
	inline point_t Checked(point_t p) {
		if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) throw std::runtime_error("non-finite transformed coordinate");
		return p;
	}
	struct sTransform {
		std::array<point_t, 3> axes{point_t{1.,0.,0.}, point_t{0.,1.,0.}, point_t{0.,0.,1.}};
		point_t offset;
		point_t Vector(point_t p) const { return Checked(Add(Add(Scale(axes[0], p.x), Scale(axes[1], p.y)), Scale(axes[2], p.z))); }
		point_t Point(point_t p) const { return Checked(Add(Vector(p), offset)); }
	};
	inline sTransform Compose(sTransform const& outer, sTransform const& inner) {
		return {{outer.Vector(inner.axes[0]), outer.Vector(inner.axes[1]), outer.Vector(inner.axes[2])}, outer.Point(inner.offset)};
	}
	inline sTransform OCS(point_t normal) {
		double length = Length(normal);
		if (!(length > 0.) || !std::isfinite(length)) throw std::runtime_error("invalid extrusion normal");
		normal = Scale(normal, 1./length);
		auto x = Cross((std::abs(normal.x) < 1./64 && std::abs(normal.y) < 1./64) ? point_t{0.,1.,0.} : point_t{0.,0.,1.}, normal);
		x = Scale(x, 1./Length(x));
		return {{x, Cross(normal, x), normal}, {}};
	}
	inline sTransform Insertion(point_t position, point_t scale, double rotation, point_t normal, point_t base, double column, double row) {
		double c = std::cos(rotation), s = std::sin(rotation);
		sTransform rotated{{point_t{c,s,0.}, point_t{-s,c,0.}, point_t{0.,0.,1.}}, {}};
		sTransform local{{Scale(rotated.axes[0], scale.x), Scale(rotated.axes[1], scale.y), Scale(rotated.axes[2], scale.z)}, {}};
		// Array spacing is rotated with the INSERT, but is not multiplied by its scale.
		local.offset = Checked(Add(Add(position, rotated.Vector({column,row,0.})), Scale(local.Vector(base), -1.)));
		return Compose(OCS(normal), local);
	}
	inline bool Planar(sTransform const& transform) {
		double scale = std::max(Length(transform.axes[0]), Length(transform.axes[1]));
		return scale > 0. && std::abs(transform.axes[0].z) <= scale * 1e-12 && std::abs(transform.axes[1].z) <= scale * 1e-12;
	}
	inline bool Similarity(sTransform const& transform) {
		double a = Length(transform.axes[0]), b = Length(transform.axes[1]);
		if (!(a > 0.) || !(b > 0.)) return false;
		return std::abs(a-b) <= std::max(a,b)*1e-12 && std::abs(Dot(Scale(transform.axes[0],1./a), Scale(transform.axes[1],1./b))) <= 1e-12;
	}
}

//////////////////////////////////////////////////////////////////////
//
// shape_default.h:
//
// 2021.05.28
// PWH
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <deque>
#include "gtl/unit.h"
#include "gtl/coord.h"
#include "gtl/dynamic.h"

#include "boost/ptr_container/ptr_container.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"

//export module shape;

namespace gtl {

	template < typename archive >
	void serialize(archive& ar, gtl::mm_t& len, unsigned int const file_version) {
		ar & len.dValue;
	}

	template < typename archive >
	void serialize(archive& ar, gtl::deg_t& len, unsigned int const file_version) {
		ar & len.dValue;
	}

	template < typename archive >
	void serialize(archive& ar, gtl::rad_t& len, unsigned int const file_version) {
		ar & len.dValue;
	}

};

namespace gtl::shape {

	using namespace gtl::literals;

	using point_t = CPoint3d;
	struct line_t { point_t beg, end; };
	struct polypoint_t : public TPointT<double, 4> {
		double& Bulge() { return w; }
		double Bulge() const { return w; }
	};

	using color_t = color_rgba_t;

	enum class eSHAPE : uint8_t { none, layer, dot, line, polyline, spline, circle, arc, ellipse, text, mtext, nSHAPE };
	constexpr color_t const CR_DEFAULT = RGBA(255, 255, 255);

	//struct hatching_t {
	//	uint32_t eFlag{};
	//	double dInterval{};
	//};
	struct cookie_t {
		void* ptr{};
		std::vector<uint8_t> buffer;
		std::u8string str;
		std::chrono::nanoseconds duration;

		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (ptrdiff_t&)ptr;
			ar & buffer;
			ar & str;
			ar & duration;
		};
	};

	class ICanvas;

	struct s_shape {
		color_t color;
		//std::optional<hatching_t> hatch;
		std::optional<cookie_t> cookie;

		virtual ~s_shape() {}

		GTL__DYNAMIC_VIRTUAL_INTERFACE(s_shape);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & color;
			//ar & hatch;
			ar & cookie;
		}

		//virtual point_t PointAt(double t) const = 0;
		virtual void FlipX() = 0;
		virtual void FlipY() = 0;
		virtual void FlipZ() = 0;
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded /*= ct.IsRightHanded()*/) = 0;
		virtual bool GetBoundingRect(CRect2d&) const = 0;

		virtual void Draw(ICanvas& canvas) const = 0;
	};

}


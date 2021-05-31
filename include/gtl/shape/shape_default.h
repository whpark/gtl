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

#include "gtl/_config.h"
#include "gtl/concepts.h"
#include "gtl/unit.h"
#include "gtl/coord.h"
#include "gtl/dynamic.h"
#include "gtl/shape/_lib_gtl_shape.h"
#include "gtl/json_proxy.h"

#include "boost/json.hpp"
#include "boost/ptr_container/ptr_container.hpp"
#include "boost/ptr_container/serialize_ptr_deque.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/optional.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/export.hpp"

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

	using string_t = std::wstring;
	using point_t = CPoint3d;
	struct line_t { point_t beg, end; };
	struct polypoint_t : public TPointT<double, 4> {
		double& Bulge() { return w; }
		double Bulge() const { return w; }

		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (TPointT<double, 4>&)*this;
		}
	};

	template < typename tpoint_t >
	void FromJson_ShapeCoord(gtl::bjson& j, tpoint_t& pt) {
		pt.x = j[0];
		pt.y = j[1];
		pt.z = j[2];
	}

	using color_t = color_rgba_t;

	enum class eSHAPE : uint8_t { none, layer, dot, line, polyline, spline, circle, arc, ellipse, text, mtext, nSHAPE };
	constexpr color_t const CR_DEFAULT = RGBA(255, 255, 255);

	template < typename archive >
	void serialize(archive& ar, color_t& cr, unsigned int const file_version) {
		ar & cr.cr;
	}
	template < typename archive >
	archive& operator & (archive& ar, color_t& cr) {
		ar & cr.cr;
		return ar;
	}

	//struct hatching_t {
	//	uint32_t eFlag{};
	//	double dInterval{};
	//};
	struct cookie_t {
		void* ptr{};
		std::vector<uint8_t> buffer;
		string_t str;
		std::chrono::nanoseconds duration;

		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (ptrdiff_t&)ptr;
			ar & buffer;
			ar & str;
			decltype(duration)::rep count{};
			if constexpr (archive::is_saving()) {
				count = duration.count();
			}
			ar & count;
			
			if constexpr (archive::is_loading()) {
				duration = std::chrono::nanoseconds(count);
			}
		};
	};

	class ICanvas;

	/// @brief shape interface class
	struct GTL_SHAPE_CLASS s_shape {
		color_t color;
		//std::optional<hatching_t> hatch;
		boost::optional<cookie_t> cookie;

		virtual ~s_shape() {}

		GTL__DYNAMIC_VIRTUAL_INTERFACE(s_shape);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & color;
			ar & cookie;
		}
		virtual bool FromJson(boost::json::value& _j, string_t& layer) {
			using namespace std::literals;
			bjson j(_j);
			color.cr = (int)j["color"sv];
			layer = j["layer"sv];
			return true;
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


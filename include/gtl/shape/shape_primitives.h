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
#include <array>
#include <vector>
#include <deque>
#include <optional>

#include "gtl/_config.h"
#include "gtl/concepts.h"
#include "gtl/unit.h"
#include "gtl/coord.h"
#include "gtl/dynamic.h"
#include "gtl/json_proxy.h"

#include "gtl/shape/_lib_gtl_shape.h"

#include "boost/json.hpp"
#include "boost/variant.hpp"
#include "boost/ptr_container/ptr_deque.hpp"
#include "boost/ptr_container/serialize_ptr_deque.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/optional.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/variant.hpp"
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
#pragma pack(push, 8)

	using namespace gtl::literals;

	using char_t = wchar_t;
	using string_t = std::wstring;
	using point_t = CPoint3d;
	struct line_t { point_t beg, end; };
	using json_t = boost::json::value;

	struct polypoint_t : public TPointT<double, 4> {
		double& Bulge() { return w; }
		double Bulge() const { return w; }

		template < typename archive >
		friend void serialize(archive& ar, polypoint_t& var, unsigned int const file_version) {
			ar & (TPointT<double, 4>&)var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, polypoint_t& var) {
			ar & (TPointT<double, 4>&)var;
		}
	};

	inline point_t PointFrom(gtl::bjson<json_t> j) {
		point_t pt;
		pt.x = j[0];
		pt.y = j[1];
		pt.z = j[2];
		return pt;
	}
	inline polypoint_t PolyPointFrom(gtl::bjson<json_t> j) {
		polypoint_t pt;
		pt.x = j[0];
		pt.y = j[1];
		//pt.z = j[2];	// no z value
		pt.w = j[4];
		return pt;
	}
	inline polypoint_t PolyPointFromVertex(gtl::bjson<json_t> j) {
		using namespace std::literals;
		polypoint_t pt;
		pt = PolyPointFrom(j["basePoint"sv]);
		pt.w = j["bulge"sv];
		return pt;
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

		template < typename archive >
		friend void serialize(archive& ar, cookie_t& var, unsigned int const file_version) {
			ar & var;
		}
		template < typename archive >
		friend void serialize(archive& ar, cookie_t& var) {
			ar & (ptrdiff_t&)var.ptr;
			ar & var.buffer;
			ar & var.str;
			decltype(duration)::rep count{};
			if constexpr (archive::is_saving()) {
				count = var.duration.count();
			}
			ar & count;
			
			if constexpr (archive::is_loading()) {
				var.duration = std::chrono::nanoseconds(count);
			}
		};
	};

	struct line_type_t {
		string_t name;
		enum class eFLAG { xref = 16, xref_resolved = xref|32, modified_internal = 64 /* auto-cad internal command */ };
		int flags{};
		string_t description;
		std::vector<double> path;

		template < typename archive >
		friend void serialize(archive& ar, line_type_t& var, unsigned int const file_version) {
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, line_type_t& var) {
			ar & var.name;
			ar & var.flags;
			ar & var.path;
		}

	};

#if 1
	using variable_t = boost::variant<string_t, int, double, point_t>;
#else
	struct variable_t {
		string_t name;
		std::variant<string_t, int, double, point_t> content;

		template < typename archive >
		friend void serialize(archive& ar, variable_t& var, unsigned int const file_version) {
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, variable_t& var) {
			ar & var.name;
			int index{};
			if constexpr (archive::is_saving()) {
				index = (int)var.content.index();
			}
			ar & index;
			if constexpr (!archive::is_saving()) {
				switch (index) {
				case 0 : var.content.emplace<0>({}); break;
				case 1 : var.content.emplace<1>({}); break;
				case 2 : var.content.emplace<2>({}); break;
				case 3 : var.content.emplace<3>({}); break;
				}
			}

			switch (index) {
			case 0 : ar & std::get<0>(var.content); break;
			case 1 : ar & std::get<1>(var.content); break;
			case 2 : ar & std::get<2>(var.content); break;
			case 3 : ar & std::get<3>(var.content); break;
			}
			return ar;
		}

	};
#endif


	class ICanvas;

	/// @brief shape interface class
	struct GTL_SHAPE_CLASS s_shape {
	public:
		enum class eTYPE {
			none = -1,
			e3dface = 0,
			arc_xy,
			block,
			circle_xy,
			dimension,
			dimaligned,
			dimlinear,
			dimradial,
			dimdiametric,
			dimangular,
			dimangular3p,
			dimordinate,
			ellipse_xy,
			hatch,
			image,
			insert,
			leader,
			line,
			lwpolyline,
			mtext,
			dot,
			polyline,
			ray,
			solid,
			spline,
			text,
			trace,
			underlay,
			vertex,
			viewport,
			xline,

			layer = 127,
			drawing = 128,

		};
	protected:
		int crIndex{};	// 0 : byblock, 256 : bylayer, negative : layer is turned off (optional)
	public:
		color_t color{};
		int eLineType{};
		string_t strLineType;
		int lineWeight{1};
		bool bVisible{};
		bool bTransparent{};
		//std::optional<hatching_t> hatch;
		boost::optional<cookie_t> cookie;

	public:
		virtual ~s_shape() {}

		GTL__DYNAMIC_VIRTUAL_INTERFACE(s_shape);
		template < typename archive >
		friend void serialize(archive& ar, s_shape& var, unsigned int const file_version) {
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_shape& var) {
			return ar & var.color & var.cookie & var.strLineType & var.lineWeight & var.eLineType & var.bVisible & var.bTransparent;
		}
		virtual bool FromJson(json_t& _j, string_t& layer);

		virtual eTYPE GetType() const = 0;
		//virtual point_t PointAt(double t) const = 0;
		virtual void FlipX() = 0;
		virtual void FlipY() = 0;
		virtual void FlipZ() = 0;
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded /*= ct.IsRightHanded()*/) = 0;
		virtual bool GetBoundingRect(CRect2d&) const = 0;

		virtual void Draw(ICanvas& canvas) const = 0;

		static std::unique_ptr<s_shape> CreateShapeFromName(std::string const& strEntityName);
	};


#pragma pack(pop)
}

#include "gtl/shape/color_table.h"

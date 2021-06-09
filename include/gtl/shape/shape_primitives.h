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

#include "fmt/ostream.h"

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
	using rect_t = CRect3d;
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

	constexpr color_t const CR_DEFAULT = ColorRGBA(255, 255, 255);

	enum class eSHAPE {
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

		user_defined_1 = 256,
		user_defined_2,
		user_defined_3,
		user_defined_4,
		user_defined_5,
		user_defined_6,
		user_defined_7,
		user_defined_8,
		user_defined_9,
		user_defined_10,
		user_defined_11,
		user_defined_12,
	};

	using variable_t = boost::variant<string_t, int, double, point_t>;

	struct cookie_t {
		void* ptr{};
		std::vector<uint8_t> buffer;
		string_t str;
		std::chrono::nanoseconds duration{};

		auto operator <=> (cookie_t const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, cookie_t& var, unsigned int const file_version) {
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, cookie_t& var) {
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
			return ar;
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

			return ar;
		}

	};


	class ICanvas;

	/// @brief shape interface class
	struct GTL_SHAPE_CLASS s_shape {
	public:
	protected:
		friend struct s_drawing;
		int crIndex{};	// 0 : byblock, 256 : bylayer, negative : layer is turned off (optional)
	public:
		string_t strLayer;	// temporary value. (while loading from dxf)
		color_t color{};
		int eLineType{};
		string_t strLineType;
		int lineWeight{1};
		bool bVisible{};
		bool bTransparent{};
		//std::optional<hatching_t> hatch;
		boost::optional<cookie_t> cookie;
		//cookie_t cookie;

	public:
		virtual ~s_shape() {}

		GTL__DYNAMIC_VIRTUAL_INTERFACE(s_shape);
		//GTL__REFLECTION_VIRTUAL_BASE(s_shape);
		//GTL__REFLECTION_MEMBERS(color, eLineType, strLineType, lineWeight, bVisible, bTransparent, cookie);

		auto operator <=> (s_shape const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_shape& var, unsigned int const file_version) {
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_shape& var) {
			ar & var.color;
			ar & var.cookie;
			ar & var.strLineType;
			ar & var.lineWeight;
			ar & var.eLineType;
			ar & var.bVisible;
			ar & var.bTransparent;
			return ar;
		}
		virtual bool LoadFromCADJson(json_t& _j);

		virtual eSHAPE GetShapeType() const = 0;
		string_t const& GetShapeName() const { return GetShapeName(GetShapeType()); }
		static string_t const& GetShapeName(eSHAPE eType);

		//virtual point_t PointAt(double t) const = 0;
		virtual void FlipX() = 0;
		virtual void FlipY() = 0;
		virtual void FlipZ() = 0;
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded /*= ct.IsRightHanded()*/) = 0;
		virtual bool UpdateBoundary(rect_t&) const = 0;

		virtual void Draw(ICanvas& canvas) const;
		virtual bool DrawROI(ICanvas& canvas, rect_t const& rectROI) const;

		virtual void PrintOut(std::wostream& os) const {
			fmt::print(os, L"Type:{} - Color({:02x},{:02x},{:02x}), {}{}", GetShapeName(GetShapeType()), color.r, color.g, color.b, !bVisible?L"Invisible ":L"", bTransparent?L"Transparent ":L"");
			fmt::print(os, L"lineType:{}, lineWeight:{}\n", strLineType, lineWeight);
		}

		static std::unique_ptr<s_shape> CreateShapeFromEntityName(std::string const& strEntityName);
	};


#pragma pack(pop)
}

#include "gtl/shape/color_table.h"

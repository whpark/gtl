//////////////////////////////////////////////////////////////////////
//
// shape_primitives.h:
//
// PWH
// 2021.05.28
// 2021.06.24. module
// 2021.08.11 Compare
//
///////////////////////////////////////////////////////////////////////////////

module;

#include <cstdint>
#include <array>
#include <vector>
#include <deque>
#include <optional>
#include <chrono>

#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include "fmt/xchar.h"
#include "fmt/ostream.h"

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

#include "gtl/_config.h"
#include "gtl/_macro.h"


export module gtl.shape:primitives;
import :color_table;
import gtl;

export namespace gtl::shape {

	using namespace gtl::literals;

	using char_t = wchar_t;
	using string_t = std::wstring;
	using point_t = xPoint3d;
	using rect_t = xRect3d;
	struct line_t { point_t beg, end; };
	using json_t = boost::json::value;

	template < typename T >
	struct tpolypoint_t : public TPointT<T, 4> {
		using base_t = TPointT<T, 4>;

		using base_t::base_t;

		T& Bulge() { return base_t::w; }
		T Bulge() const { return base_t::w; }

		auto operator <=> (tpolypoint_t const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, tpolypoint_t& var, unsigned int const file_version) {
			ar & (base_t&)var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, tpolypoint_t& var) {
			ar & (base_t&)var;
		}
	};
	using polypoint_t = tpolypoint_t<double>;

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

	//constexpr color_t const CR_DEFAULT = ColorRGBA(255, 255, 255);

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
			decltype(var.duration)::rep count{};
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

		auto operator <=> (line_type_t const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, line_type_t& var, unsigned int const file_version) {
			ar & var.name;
			ar & var.flags;
			ar & var.path;
			if (file_version >= 1) {
				ar & var.description;
			}
		}
		template < typename archive >
		friend archive& operator & (archive& ar, line_type_t& var) {
			ar & var.name;
			ar & var.flags;
			ar & var.path;
			ar & var.description;
			return ar;
		}
	};

	class ICanvas;

	/// @brief shape interface class
	class xShape {
	protected:
		friend class xDrawing;
		mutable int m_crIndex{};		// 0 : byblock, 256 : bylayer, negative : layer is turned off (optional)
	public:
		mutable string_t m_strLayer;	// temporary value. (while loading from dxf)

	public:
		color_t m_color{};
		int m_eLineType{};
		string_t m_strLineType;
		int m_lineWeight{1};
		bool m_bVisible{true};
		bool m_bTransparent{};
		boost::optional<cookie_t> m_cookie;

	public:
		enum class eLINE_WIDTH : int {
			ByLayer = 29,
			ByBlock = 30,
			Default = 31
		};

	public:
		virtual ~xShape() {}

		GTL__DYNAMIC_VIRTUAL_INTERFACE(xShape);
		//GTL__REFLECTION_VIRTUAL_BASE(xShape);
		//GTL__REFLECTION_MEMBERS(m_color, m_eLineType, m_strLineType, m_lineWeight, m_bVisible, m_bTransparent, m_cookie);

		//auto operator <=> (xShape const&) const = default;
		bool operator == (xShape const& B) const {
			return Compare(B);
		}
		auto operator <=> (xShape const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xShape& var, unsigned int const file_version) {
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xShape& var) {
			ar & var.m_color.cr;
			ar & var.m_cookie;
			ar & var.m_strLineType;
			ar & var.m_lineWeight;
			ar & var.m_eLineType;
			ar & var.m_bVisible;
			ar & var.m_bTransparent;
			return ar;
		}
		virtual bool LoadFromCADJson(json_t& _j);

		virtual bool Compare(xShape const& B) const {
			if (GetShapeType() != B.GetShapeType())
				return false;

			return true
					and (m_color		== B.m_color)
					and (m_eLineType	== B.m_eLineType)
					and (m_strLineType	== B.m_strLineType)
					and (m_lineWeight	== B.m_lineWeight)
					and (m_bVisible		== B.m_bVisible)
					and (m_bTransparent	== B.m_bTransparent)
					and (m_cookie		== B.m_cookie)
				;
		}
		virtual eSHAPE GetShapeType() const = 0;
		string_t const& GetShapeName() const { return GetShapeName(GetShapeType()); }
		static string_t const& GetShapeName(eSHAPE eType);

		//virtual point_t PointAt(double t) const = 0;
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const = 0;
		virtual void FlipX() = 0;
		virtual void FlipY() = 0;
		virtual void FlipZ() = 0;
		virtual void Reverse() = 0;
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded /*= ct.IsRightHanded()*/) = 0;
		virtual bool UpdateBoundary(rect_t&) const = 0;
		virtual rect_t GetBoundary() const {
			rect_t rect;
			rect.SetRectEmptyForMinMax2d();
			UpdateBoundary(rect);
			return rect;
		}
		int GetLineWidthInUM() const { return GetLineWidthInUM(m_lineWeight); }
		static int GetLineWidthInUM(int lineWeight);

		virtual void Draw(ICanvas& canvas) const;
		virtual bool DrawROI(ICanvas& canvas, rect_t const& rectROI) const;

		virtual void PrintOut(std::wostream& os) const {
			fmt::print(os, L"Type:{} - Color({:02x},{:02x},{:02x}), {}{}", GetShapeName(GetShapeType()), m_color.r, m_color.g, m_color.b, !m_bVisible?L"Invisible ":L"", m_bTransparent?L"Transparent ":L"");
			fmt::print(os, L"lineType:{}, lineWeight:{}\n", m_strLineType, m_lineWeight);
		}

		static std::unique_ptr<xShape> CreateShapeFromEntityName(std::string const& strEntityName);
	};


	bool CohenSutherlandLineClip(gtl::xRect2d roi, gtl::xPoint2d& pt0, gtl::xPoint2d& pt1);

}

export BOOST_CLASS_VERSION(gtl::shape::line_type_t, 1);

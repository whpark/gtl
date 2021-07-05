//////////////////////////////////////////////////////////////////////
//
// shape_primitives.h:
//
// PWH
// 2021.05.28
// 2021.06.24. module
//
///////////////////////////////////////////////////////////////////////////////

module;

#include <cstdint>
#include <array>
#include <vector>
#include <deque>
#include <optional>
#include <chrono>

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


export module gtls:primitives;
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
	struct xShape {
	public:
	protected:
		friend struct xDrawing;
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
		enum class eLINE_WIDTH : int {
			ByLayer = 29,
			ByBlock = 30,
			Default = 31
		};

	public:
		virtual ~xShape() {}

		GTL__DYNAMIC_VIRTUAL_INTERFACE(xShape);
		//GTL__REFLECTION_VIRTUAL_BASE(xShape);
		//GTL__REFLECTION_MEMBERS(color, eLineType, strLineType, lineWeight, bVisible, bTransparent, cookie);

		auto operator <=> (xShape const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xShape& var, unsigned int const file_version) {
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xShape& var) {
			ar & var.color.cr;
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
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded /*= ct.IsRightHanded()*/) = 0;
		virtual bool UpdateBoundary(rect_t&) const = 0;
		int GetLineWidthInUM() const { return GetLineWidthInUM(lineWeight); }
		static int GetLineWidthInUM(int lineWeight);

		virtual void Draw(ICanvas& canvas) const;
		virtual bool DrawROI(ICanvas& canvas, rect_t const& rectROI) const;

		virtual void PrintOut(std::wostream& os) const {
			fmt::print(os, L"Type:{} - Color({:02x},{:02x},{:02x}), {}{}", GetShapeName(GetShapeType()), color.r, color.g, color.b, !bVisible?L"Invisible ":L"", bTransparent?L"Transparent ":L"");
			fmt::print(os, L"lineType:{}, lineWeight:{}\n", strLineType, lineWeight);
		}

		static std::unique_ptr<xShape> CreateShapeFromEntityName(std::string const& strEntityName);
	};

}

namespace gtl::shape {
	// Cohen-Sutherland clipping algorithm clips a line from
	// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
	// diagonal from (xmin, ymin) to (xmax, ymax).
		enum fOUT_CODE : uint8_t {
			INSIDE	= 0b0000,
			LEFT	= 0b0001,
			RIGHT	= 0b0010,
			BOTTOM	= 0b0100,
			TOP		= 0b1000,
		};
};

export namespace gtl::shape {

	void CohenSutherlandLineClip(gtl::xRect2d roi, gtl::xPoint2d& pt0, gtl::xPoint2d& pt1) {

		auto& x0 = pt0.x;
		auto& y0 = pt0.y;
		auto& x1 = pt1.x;
		auto& y1 = pt1.y;

		// Compute the bit code for a point (x, y) using the clip
		// bounded diagonally by (xmin, ymin), and (xmax, ymax)
		auto ComputeOutCode = [&roi](auto x, auto y) -> fOUT_CODE {
			fOUT_CODE code {};
			if (x < roi.left)
				(uint8_t&)code |= fOUT_CODE::LEFT;
			else if (x >= roi.right)
				(uint8_t&)code |= fOUT_CODE::RIGHT;
			if (y < roi.top)
				(uint8_t&)code |= fOUT_CODE::BOTTOM;
			else if (y >= roi.bottom)
				(uint8_t&)code |= fOUT_CODE::TOP;
			return code;
		};

		// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
		auto outcode0 = ComputeOutCode(x0, y0);
		auto outcode1 = ComputeOutCode(x1, y1);
		bool accept = false;

		while (true) {
			if (!(outcode0 | outcode1)) {
				// bitwise OR is 0: both points inside window; trivially accept and exit loop
				accept = true;
				break;
			} else if (outcode0 & outcode1) {
				// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
				// or BOTTOM), so both must be outside window; exit loop (accept is false)
				break;
			} else {
				// failed both tests, so calculate the line segment to clip
				// from an outside point to an intersection with clip edge
				double x, y;

				// At least one endpoint is outside the clip rectangle; pick it.
				auto outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

				// Now find the intersection point;
				// use formulas:
				//   slope = (y1 - y0) / (x1 - x0)
				//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
				//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
				// No need to worry about divide-by-zero because, in each case, the
				// outcode bit being tested guarantees the denominator is non-zero
				if (outcodeOut & fOUT_CODE::TOP) {           // point is above the clip window
					x = x0 + (x1 - x0) * (roi.bottom - y0) / (y1 - y0);
					y = roi.bottom;
				} else if (outcodeOut & fOUT_CODE::BOTTOM) { // point is below the clip window
					x = x0 + (x1 - x0) * (roi.top - y0) / (y1 - y0);
					y = roi.top;
				} else if (outcodeOut & fOUT_CODE::RIGHT) {  // point is to the right of clip window
					y = y0 + (y1 - y0) * (roi.right - x0) / (x1 - x0);
					x = roi.right;
				} else if (outcodeOut & fOUT_CODE::LEFT) {   // point is to the left of clip window
					y = y0 + (y1 - y0) * (roi.left - x0) / (x1 - x0);
					x = roi.left;
				}

				// Now we move outside point to intersection point to clip
				// and get ready for next pass.
				if (outcodeOut == outcode0) {
					x0 = x;
					y0 = y;
					outcode0 = ComputeOutCode(x0, y0);
				} else {
					x1 = x;
					y1 = y;
					outcode1 = ComputeOutCode(x1, y1);
				}
			}
		}
	}

	int xShape::GetLineWidthInUM(int lineWeight) {
		static int const widths[] = {
			0,
			50,
			90,
			130,
			150,
			180,
			200,
			250,
			300,
			350,
			400,
			500,
			530,
			600,
			700,
			800,
			900,
			1000,
			1060,
			1200,
			1400,
			1580,
			2000,
			2110,
		};

		if ( (lineWeight >= 0) and (lineWeight < std::size(widths)) ) {
			return widths[lineWeight];
		}

		return 0;
	};

}

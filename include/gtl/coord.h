#pragma once

//////////////////////////////////////////////////////////////////////
//
// coord.h:	좌표 관련 struct.
//
// PWH
// 2018.12.05.
// 2019.07.24. QL -> GTL
// 2021.05.18.
//
//////////////////////////////////////////////////////////////////////

#include "gtl/misc.h"
#include "gtl/concepts.h"

//#include "boost/serialization/serialization.hpp"

#include "gtl/coord/coord_size.h"
#include "gtl/coord/coord_point.h"
#include "gtl/coord/coord_rect.h"
#include "gtl/coord/coord_srect.h"
#include "gtl/coord/coord_trans.h"

//=============================================================================
//
// 참고 : 3차원 변수에 2차원 변수를 대입할 경우, Z 축 은 값을 변경시키지 않도록 했음.
//       (ex, XPoint3d pt3(1, 2, 3); XPoint2d pt2(4, 5); pt3 = pt2; assert(pt3 == XPoint3d(4, 5, 3)); )
//


namespace gtl {
#pragma pack(push, 8)


	//=============================================================================
	//
	// 참고 : 3차원 변수에 2차원 변수를 대입할 경우, Z 축 은 값을 변경시키지 않도록 했음.
	//       (ex, XPoint3d pt3(1, 2, 3); CQPoint2d pt2(4, 5); pt3 = pt2; assert(pt3 == CQPoint3d(4, 5, 3)); )
	//


	using xSize3d	= TSize3<double>;
	using xSize2d	= TSize2<double>;
	using xPoint4d	= TPoint4<double>;
	using xPoint3d	= TPoint3<double>;
	using xPoint2d	= TPoint2<double>;
	using xRect3d	= TRect3<double>;
	using xRect2d	= TRect2<double>;
	using xSRect3d	= TSRect3<double>;
	using xSRect2d	= TSRect2<double>;
	using xSize3i	= TSize3<int>;
	using xSize2i	= TSize2<int>;
	using xPoint4i	= TPoint4<int>;
	using xPoint3i	= TPoint3<int>;
	using xPoint2i	= TPoint2<int>;
	using xRect3i	= TRect3<int>;
	using xRect2i	= TRect2<int>;
	using xSRect3i	= TSRect3<int>;
	using xSRect2i	= TSRect2<int>;


	/// @brief Interpolation (lerp)
	template < typename T, int dim >
	TPointT<T, dim> lerp(TPointT<T, dim> const& a, TPointT<T, dim> const& b, double t) {
		TPointT<T, dim> c;
		for (int i = 0; i < dim; i++)
			c.member(i) = std::lerp(a.member(i), b.member(i), t);
		return c;
	}


	//-------------------------------------------------------------------------
	// Archive <---------> TPoint2/3, TSize2/3, TRect2/3
	//
	template < typename Archive, gtlc::coord T_COORD >
	Archive& Coord2Archive(const T_COORD& coord, Archive& ar) {
		for (const auto& v : coord.data())
			ar & v;
		return ar;
	}

	template < typename Archive, gtlc::coord T_COORD >
	Archive& Archive2Coord(Archive& ar, T_COORD& coord) {
		for (auto& v : coord.data())
			ar & v;
		return ar;
	}

	template < typename Archive, gtlc::coord T_COORD >
	Archive& SerializeCoord(Archive& ar, T_COORD& coord) {
		return ar.IsStoring() ? Coord2Archive<Archive, T_COORD>(coord, ar) : Archive2Coord<Archive, T_COORD>(ar, coord);
	}

	//-------------------------------------------------------------------------
	// to Text, From Text

	template < gtlc::string_elem tchar, gtlc::arithmetic tvalue >
	constexpr fmt::basic_format_string<tchar, tvalue> GetDefaultFormatSpecifier() {
		using namespace std::literals;
		if constexpr (std::is_same_v<tchar, char>) {
			return "{}";
		}
		else if constexpr (std::is_same_v<tchar, wchar_t>) {
			return L"{}";
		}
		else if constexpr (std::is_same_v<tchar, char8_t>) {
			return u8"{}";
		}
		else if constexpr (std::is_same_v<tchar, char16_t>) {
			return u"{}";
		}
		else if constexpr (std::is_same_v<tchar, char32_t>) {
			return U"{}";
		}
		else {
			static_assert(gtlc::dependent_false_v);
		}
	}

	//-------------------------------------------------------------------------
	// to Text, From Text
	template < typename tchar, gtlc::coord T_COORD >
	std::basic_string<tchar> ToString(T_COORD const& coord, fmt::basic_format_string<tchar, typename T_COORD::value_type> const& svFMT = GetDefaultFormatSpecifier<tchar, typename T_COORD::value_type>()) {
		std::basic_string<tchar> str;
		//if ( !(fmt::basic_string_view<tchar>(svFMT)).data() )
		//	svFMT = GetDefaultFormatSpecifier<tchar, typename T_COORD::value_type>();

		for (size_t i = 0; i < coord.size(); i++) {
			if (i)
				str += ',';
			str += fmt::vformat((fmt::basic_string_view<tchar>)svFMT, fmt::make_format_args<fmt::buffer_context<tchar>>(coord.data()[i]));
		}
		return str;
	}
	template < gtlc::coord T_COORD, typename tchar >
	T_COORD FromString(std::basic_string_view<tchar> sv) {
		T_COORD coord;
		tchar const* pos = sv.data();
		tchar const* const end = sv.data() + sv.size();
		for (auto& v : coord.arr()) {
			if (pos >= end)
				break;
			v = tszto<typename T_COORD::value_type>(pos, end, &pos);
			if (*pos == ',') pos++;	// or....... if (*pos) pos++
		}
		return coord;
	}


	//-------------------------------------------------------------------------
	// Check ROI

	template < gtlc::cv_rect T_CV_RECT, gtlc::cv_size T_CV_SIZE >
	bool IsROI_Valid(T_CV_RECT const& rcROI, T_CV_SIZE const& sizeImage) {
		return 1
			&& ( (rcROI.x >= 0) && (rcROI.y >= 0) )
			&& ( (rcROI.width > 0) && (rcROI.height > 0) )
			&& ( (sizeImage.width < 0) || ((rcROI.x < sizeImage.width) && (rcROI.x+rcROI.width <= sizeImage.width)) )
			&& ( (sizeImage.height < 0) || ((rcROI.y < sizeImage.height) && (rcROI.y+rcROI.height <= sizeImage.height)) )
			;
	};

	template < gtlc::cv_rect T_CV_RECT, gtlc::cv_size T_CV_SIZE >
	[[nodiscard]] T_CV_RECT GetSafeROI(const T_CV_RECT& rc, const T_CV_SIZE& sizeImage) {
		T_CV_RECT rcSafe(rc);

		if (rcSafe.width < 0) {
			rcSafe.x += rcSafe.width;
			rcSafe.width = -rcSafe.width;
		}
		if (rcSafe.height < 0) {
			rcSafe.y += rcSafe.height;
			rcSafe.height = -rcSafe.height;
		}

		if (rcSafe.x < 0)
			rcSafe.x = 0;
		if (rcSafe.y < 0)
			rcSafe.y = 0;

		// Error
		if ( (rcSafe.x >= sizeImage.width) || (rcSafe.y >= sizeImage.height) ) {
			rcSafe = T_CV_RECT{};
			return rcSafe;
		}
		if ( (sizeImage.width > 0) && (rcSafe.x + rcSafe.width > sizeImage.width) )
			rcSafe.width = sizeImage.width - rcSafe.x;
		if ( (sizeImage.height > 0) && (rcSafe.y + rcSafe.height > sizeImage.height) )
			rcSafe.height = sizeImage.height - rcSafe.y;

		// Error
		if ( (sizeImage.width <= 0) || (sizeImage.height <= 0) ) {
			rcSafe = T_CV_RECT{};
			return rcSafe;
		}

		return rcSafe;
	};


#if 0	// obsolete
	template < int iMember, gtlc::coord T_COORD >
	constexpr auto& GetCoordMember(T_COORD& B) {
		if constexpr (gtl::has__xy<T_COORD>) {
			if constexpr (iMember == 0) return B.x;
			else if constexpr (iMember == 1) return B.y;
			else if constexpr ((iMember == 2) && has__z<T_COORD>) return B.z;
			else static_assert(gtlc::dependent_false_v);
		} else if constexpr (gtl::has__cxy<T_COORD>) {
			if constexpr (iMember == 0) return B.cx;
			else if constexpr (iMember == 1) return B.cy;
			else if constexpr ((iMember == 2) && has__cz<T_COORD>) return B.cz;
		} else if constexpr (gtl::has__size2<T_COORD>) {
			if constexpr (iMember == 0) return B.width;
			else if constexpr (iMember == 1) return B.height;
			else if constexpr ((iMember == 2) && has__depth<T_COORD>) return B.depth;
		} else static_assert(gtlc::dependent_false_v);
	};
	template < int iMember, gtl::is__coord T_COORD >
	constexpr const auto& GetCoordMember(const T_COORD& B) {
		if constexpr (gtl::has__xy<T_COORD>) {
			if constexpr (iMember == 0) return B.x;
			else if constexpr (iMember == 1) return B.y;
			else if constexpr ((iMember == 2) && has__z<T_COORD>) return B.z;
			else static_assert(gtlc::dependent_false_v);
		} else if constexpr (gtl::has__cxy<T_COORD>) {
			if constexpr (iMember == 0) return B.cx;
			else if constexpr (iMember == 1) return B.cy;
			else if constexpr ((iMember == 2) && has__cz<T_COORD>) return B.cz;
			else static_assert(gtlc::dependent_false_v);
		} else if constexpr (gtl::has__size2<T_COORD>) {
			if constexpr (iMember == 0) return B.width;
			else if constexpr (iMember == 1) return B.height;
			else if constexpr ((iMember == 2) && has__depth<T_COORD>) return B.depth;
			else static_assert(gtlc::dependent_false_v);
		} else static_assert(gtlc::dependent_false_v);
	};
	template < int iMember, gtl::is__coord T_COORD >
	constexpr bool HasCoordMember(const T_COORD& B) {
		if constexpr (gtl::has__xy<T_COORD>) {
			if constexpr (iMember == 0) return true;
			else if constexpr (iMember == 1) return true;
			else if constexpr ((iMember == 2) && has__z<T_COORD>) return true;
			else return false;
		} else if constexpr (gtl::has__cxy<T_COORD>) {
			if constexpr (iMember == 0) return true;
			else if constexpr (iMember == 1) return true;
			else if constexpr ((iMember == 2) && has__cz<T_COORD>) return true;
			else return false;
		} else if constexpr (gtl::has__size2<T_COORD>) {
			if constexpr (iMember == 0) return true;
			else if constexpr (iMember == 1) return true;
			else if constexpr ((iMember == 2) && has__depth<T_COORD>) return true;
			else return false;
		} else return false;
	};
#endif


#pragma pack(pop)
}	// namespace gtl

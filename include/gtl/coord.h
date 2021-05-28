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

#include "boost/archive/polymorphic_xml_iarchive.hpp"
#include "boost/archive/polymorphic_xml_oarchive.hpp"

#include "gtl/coord/size.h"
#include "gtl/coord/point.h"
#include "gtl/coord/rect.h"
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


	using CSize3d	= TSize3<double>;
	using CSize2d	= TSize2<double>;
	using CPoint3d	= TPoint3<double>;
	using CPoint2d	= TPoint2<double>;
	using CRect3d	= TRect3<double>;
	using CRect2d	= TRect2<double>;
	using CSize3i	= TSize3<int>;
	using CSize2i	= TSize2<int>;
	using CPoint3i	= TPoint3<int>;
	using CPoint2i	= TPoint2<int>;
	using CRect3i	= TRect3<int>;
	using CRect2i	= TRect2<int>;


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
	template < typename tchar_t, gtlc::coord T_COORD >
	std::basic_string<tchar_t> ToString(const T_COORD& coord, const tchar_t* pszFMT = nullptr) {
		std::basic_string<tchar_t> str;
		if ( !pszFMT || !pszFMT[0] )
			pszFMT = GetDefaultFormatSpecifier<tchar_t, typename T_COORD::value_type>(0);

		for (size_t i = 0; i < coord.data().size(); i++) {
			if (i) {
				str += ',';
				str += ' ';
			}
			str += Format(pszFMT, coord.data()[i]);
		}
		return str;
	}
	template < typename tchar_t, gtlc::coord T_COORD >
	std::optional<T_COORD> FromString(const tchar_t* psz) {
		T_COORD coord;
		for (auto& v : coord.data()) {
			if (!psz || !*psz)
				return nullptr;
			v = tszto_number<tchar_t, typename T_COORD::value_type>(psz, &psz);
			if (*psz == ',') psz++;	// or....... if (*psz) psz++
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
			else static_assert(false);
		} else if constexpr (gtl::has__cxy<T_COORD>) {
			if constexpr (iMember == 0) return B.cx;
			else if constexpr (iMember == 1) return B.cy;
			else if constexpr ((iMember == 2) && has__cz<T_COORD>) return B.cz;
		} else if constexpr (gtl::has__size2<T_COORD>) {
			if constexpr (iMember == 0) return B.width;
			else if constexpr (iMember == 1) return B.height;
			else if constexpr ((iMember == 2) && has__depth<T_COORD>) return B.depth;
		} else static_assert(false);
	};
	template < int iMember, gtl::is__coord T_COORD >
	constexpr const auto& GetCoordMember(const T_COORD& B) {
		if constexpr (gtl::has__xy<T_COORD>) {
			if constexpr (iMember == 0) return B.x;
			else if constexpr (iMember == 1) return B.y;
			else if constexpr ((iMember == 2) && has__z<T_COORD>) return B.z;
			else static_assert(false);
		} else if constexpr (gtl::has__cxy<T_COORD>) {
			if constexpr (iMember == 0) return B.cx;
			else if constexpr (iMember == 1) return B.cy;
			else if constexpr ((iMember == 2) && has__cz<T_COORD>) return B.cz;
			else static_assert(false);
		} else if constexpr (gtl::has__size2<T_COORD>) {
			if constexpr (iMember == 0) return B.width;
			else if constexpr (iMember == 1) return B.height;
			else if constexpr ((iMember == 2) && has__depth<T_COORD>) return B.depth;
			else static_assert(false);
		} else static_assert(false);
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

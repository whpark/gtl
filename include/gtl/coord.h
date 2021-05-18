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

#include "gtl/coord/size.h"
#include "gtl/coord/point.h"
#include "gtl/coord/rect.h"

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


	//-------------------------------------------------------------------------
	// Compare Coord values
	namespace internal {
		//// array 값을 대입.
		//template < typename T_DEST, int nDest, typename T_SOURCE, int nSource, class = std::is_pod<T_DEST>, class = std::is_pod<T_SOURCE> >
		//void AssignPODArray(std::array<T_DEST, nDest>& dest, const std::array<T_SOURCE, nSource>& src) {
		//	if constexpr (std::is_same_v<T_DEST, T_SOURCE>) {
		//		// 타입이 동일할 경우
		//		if constexpr (nDest == nSource) {
		//			dest = src;
		//		} else {
		//			memcpy(dest.data(), src.data(), std::min(sizeof(dest), sizeof(src)));
		//		}
		//	} else {
		//		// 일반적인 경우
		//		for (int i = 0; i < std::min(dest.size(), src.size()); i++)
		//			dest[i] = RoundOrForward<T_DEST, T_SOURCE>(src[i]);
		//	}
		//}
		// 비교 함수
		template < typename T1, int n1, typename T2, int n2, class Pred >
		bool CompareCoordValues(const std::array<T1, n1>& A, const std::array<T2, n2>& B, Pred pred) {
			for (int i = 0; i < std::min(A.size(), B.size()); i++)
				if (!pred(A[i], B[i]))
					return false;
			return true;
		}
		template < typename T1, int n1, typename T2, class Pred >
		bool CompareCoordValues(const std::array<T1, n1>& A, T2 b, Pred pred) {
			for (int i = 0; i < n1; i++)
				if (!pred(A[i], b))
					return false;
			return true;
		}
	}

	template < typename T1, typename T2, template <typename> class T_COORD1, template <typename> class T_COORD2,
		typename T = std::conditional_t<(sizeof(T1) >= sizeof(T2)), T1, T2>,
		class = typename T_COORD1<T1>::coord_t,
		class = typename T_COORD2<T2>::coord_t
	>
	requires (std::is_floating_point_v<T1> && std::is_floating_point_v<T2>)
	bool CompareCoord(const T_COORD1<T1>& a, const T_COORD2<T2>& b, T dEpsilon) {
		return gtl::internal::CompareCoordValues(
			a.data(),
			b.data(),
			[dEpsilon](const auto& v1, const auto& v2)->bool {
				return std::abs(v1-v2) < dEpsilon;
			}
		);
	};

	template < typename T, template <typename> class T_COORD1, template <typename> class T_COORD2,
		class = typename T_COORD1<T>::coord_t
	>
	bool CompareCoord(const T_COORD1<T>& a, const T_COORD2<T>& b) {
		return gtl::internal::CompareCoordValues(a.data(), b.data(), std::equal_to<T>());
	}


	//-------------------------------------------------------------------------
	// Archive <---------> TPoint2/3, TSize2/3, TRect2/3
	//
	template < typename Archive, class T_COORD, class = typename T_COORD::coord_t >
	Archive& Coord2Archive(const T_COORD& coord, Archive& ar) {
		for (const auto& v : coord.data())
			ar & v;
		return ar;
	}

	template < typename Archive, class T_COORD, class = typename T_COORD::coord_t >
	Archive& Archive2Coord(Archive& ar, T_COORD& coord) {
		for (auto& v : coord.data())
			ar & v;
		return ar;
	}

	template < typename Archive, class T_COORD, class = typename T_COORD::coord_t >
	Archive& SerializeCoord(Archive& ar, T_COORD& coord) {
		return ar.IsStoring() ? Coord2Archive<Archive, T_COORD>(coord, ar) : Archive2Coord<Archive, T_COORD>(ar, coord);
	}


	//-------------------------------------------------------------------------
	// to Text, From Text
	template < typename tchar_t, class T_COORD, class = typename T_COORD::coord_t >
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
	template < typename tchar_t, class T_COORD, class = typename T_COORD::coord_t >
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

	template < typename T2,
		template <typename> typename T_CV_RECT,
		template <typename> typename T_CV_SIZE
	> requires (
		requires (T_CV_RECT<T2> rc, T_CV_SIZE<T2> size) {
			rc.x; rc.y; rc.width; rc.height;
			size.width; size.height;
		}
	)
	bool IsROI_Valid(const T_CV_RECT<T2>& rcROI, const T_CV_SIZE<T2>& sizeImage) {
		return 1
			&& ( (rcROI.x >= 0) && (rcROI.y >= 0) )
			&& ( (rcROI.width > 0) && (rcROI.height > 0) )
			&& ( (sizeImage.width < 0) || ((rcROI.x < sizeImage.width) && (rcROI.x+rcROI.width <= sizeImage.width)) )
			&& ( (sizeImage.height < 0) || ((rcROI.y < sizeImage.height) && (rcROI.y+rcROI.height <= sizeImage.height)) )
			;
	};

	template < typename T2,
		template <typename> typename T_CV_RECT,
		template <typename> typename T_CV_SIZE
	> requires (
		requires (T_CV_RECT<T2> rc, T_CV_SIZE<T2> size) {
			rc.x; rc.y; rc.width; rc.height;
			size.width; size.height;
		}
	)
	[[nodiscard]] T_CV_RECT<T2> GetSafeROI(const T_CV_RECT<T2>& rc, const T_CV_SIZE<T2>& sizeImage) {
		T_CV_RECT<T2> rcSafe(rc);

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
			rcSafe = T_CV_RECT<T2>();
			return rcSafe;
		}
		if ( (sizeImage.width > 0) && (rcSafe.x + rcSafe.width > sizeImage.width) )
			rcSafe.width = sizeImage.width - rcSafe.x;
		if ( (sizeImage.height > 0) && (rcSafe.y + rcSafe.height > sizeImage.height) )
			rcSafe.height = sizeImage.height - rcSafe.y;

		// Error
		if ( (sizeImage.width <= 0) || (sizeImage.height <= 0) ) {
			rcSafe = T_CV_RECT<T2>();
			return rcSafe;
		}

		return rcSafe;
	};


#pragma pack(pop)
}	// namespace gtl

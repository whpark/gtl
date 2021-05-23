#pragma once

//////////////////////////////////////////////////////////////////////
//
// coord_trans.h: coord trans
//
// PWH
// 2019.11.02. 전체 수정
// 2021.05.21. renewal
//
//////////////////////////////////////////////////////////////////////

#include "gtl/_lib_gtl.h"
#include "gtl/concepts.h"
#include "gtl/misc.h"
#include "gtl/unit.h"
#include "gtl/coord.h"
#include "gtl/dynamic.h"

#include "boost/ptr_container/ptr_deque.hpp"
#include "boost/archive/polymorphic_xml_iarchive.hpp"
#include "boost/archive/polymorphic_xml_oarchive.hpp"

#include "opencv2/opencv.hpp"

#if 1

namespace gtl {
#pragma pack(push, 8)

	//-----------------------------------------------------------------------------
	// interface CoordTrans;
	class GTL_CLASS ICoordTrans {
	public:
		using this_t = ICoordTrans;

		// Constructors
		ICoordTrans() { }
		virtual ~ICoordTrans() { }

		GTL__VIRTUAL_DYNAMIC_INTERFACE(ICoordTrans);

	public:
		friend class boost::serialization::access;
		template < typename Archive >
		void serialize(Archive &ar, unsigned int const version) {
			//ar & boost::serialization::base_object<base_t>(*this);
		}

	public:
		/// @brief pt -> pt2
		/// @return 
		CPoint3d operator () (CPoint3d const& pt) const {
			return Trans(pt);
		}

		/// @brief 
		/// @tparam tcoord3d : Not CPoint3d but one of 3d Coord.
		/// @param pt 
		/// @return 
		template < typename tcoord3d > requires (gtlc::tcoord3<tcoord3d, double> and !std::is_same_v<CPoint3d, tcoord3d>)
		[[ nodiscard ]] tcoord3d operator () (tcoord3d const& pt) const {
			auto v{pt};
			((CPoint3d&)v) = Trans((CPoint3d&)pt);
			return v;
		}

		/// @brief tcoord3d : not a Coord, buf has x, y, z
		template < typename tcoord3d > requires (gtlc::has__xyz<tcoord3d>) and (!gtlc::tcoord3<tcoord3d, double>)
		[[ nodiscard ]] tcoord3d operator () (tcoord3d const& pt) const {
			auto v{pt};
			auto r = Trans(CPoint3d(pt.x, pt.y, pt.z));
			v.x = RoundOrForward<decltype(v.x)>(r.x);
			v.y = RoundOrForward<decltype(v.y)>(r.y);
			v.z = RoundOrForward<decltype(v.z)>(r.z);
			return v;
		}


		/// @brief pt -> pt2
		/// @param pt 
		/// @return 
		[[ nodiscard ]] CPoint2d operator () (CPoint2d const& pt) const {
			return Trans(pt);
		}

		/// @brief pt -> pt2
		/// @tparam tcoord3d : Not CPoint3d but one of 3d Coord.
		/// @param pt 
		/// @return 
		template < typename tcoord2d > requires (gtlc::tcoord2<tcoord2d, double> and !std::is_same_v<CPoint2d, tcoord2d>)
		[[ nodiscard ]] tcoord2d operator () (tcoord2d const& pt) const {
			auto v{pt};
			((CPoint2d&)v) = Trans((CPoint2d&)pt);
			return v;
		}

		/// @brief tcoord2d : not a Coord, buf has x, y
		template < typename tcoord2d > requires (gtlc::has__xy<tcoord2d>) and (!gtlc::tcoord2<tcoord2d, double>)
		[[ nodiscard ]] tcoord2d operator () (tcoord2d const& pt) const {
			auto v{pt};
			auto r = Trans(CPoint2d(pt.x, pt.y));
			v.x = RoundOrForward<decltype(v.x)>(r.x);
			v.y = RoundOrForward<decltype(v.y)>(r.y);
			return v;
		}

		//-------------------------------------------------------------------------
		// Operation
		//
		virtual CPoint2d Trans(CPoint2d const& pt) const = 0;
		virtual CPoint2d TransI(CPoint2d const& pt) const = 0;
		virtual CPoint3d Trans(CPoint3d const& pt) const = 0;
		virtual CPoint3d TransI(CPoint3d const& pt) const = 0;
		virtual double Trans(double dLength) const = 0;
		virtual double TransI(double dLength) const = 0;

		virtual bool IsRightHanded() const = 0;
	};


	//-----------------------------------------------------------------------------
	class GTL_CLASS CCoordTransChain : public ICoordTrans {
	protected:
		boost::ptr_deque<ICoordTrans> chain_;	// 마지막 Back() CT부터 Front() 까지 Transform() 적용.

	public:
		using base_t = ICoordTrans;

		friend class boost::serialization::access;
		template < typename tBoostArchive >
		void serialize(tBoostArchive &ar, unsigned int const version) {
			ar & boost::serialization::base_object<base_t>(*this);
			ar & chain_;
		}

		GTL__VIRTUAL_DYNAMIC_DERIVED(CCoordTransChain);

	public:
		// Constructors
		CCoordTransChain() = default;
		//virtual ~CCoordTransChain() { }
		CCoordTransChain(CCoordTransChain const& B) = default;
		CCoordTransChain& operator = (CCoordTransChain const& B) = default;

		CCoordTransChain& operator *= (CCoordTransChain const& B)	{
			for (auto const& ct : chain_) 
				chain_.push_back(std::move(ct.NewClone()));
			return *this;
		}
		CCoordTransChain& operator *= (ICoordTrans const& B) {
			chain_.push_back(std::move(B.NewClone()));
			return *this;
		}
		CCoordTransChain operator * (CCoordTransChain const& B) const {
			CCoordTransChain newChain;
			for (auto const& ct : chain_) {
				newChain.chain_.push_back(std::move(ct.NewClone()));
			}
			for (auto const& ct : B.chain_) {
				newChain.chain_.push_back(std::move(ct.NewClone()));
			}
			return newChain;
		}

		void clear() { chain_.clear(); }

		//-------------------------------------------------------------------------
		// Operation
		//
		template < typename tchain, typename tpoint >
		tpoint ChainTrans(tchain const& chain, tpoint const& pt) const {
			tpoint ptT(pt);
			for (auto iter = chain.rbegin(); iter != chain.rend(); iter++)
				ptT = iter->Trans(ptT);
			return ptT;
		}
		template < typename tchain, typename tpoint >
		tpoint ChainTransI(tchain const& chain, tpoint const& pt) const {
			tpoint ptT(pt);
			for (auto iter = chain.begin(); iter != chain.end(); iter++)
				ptT = iter->TransI(ptT);
			return ptT;
		}

		virtual CPoint2d Trans(CPoint2d const& pt) const override  { return ChainTrans(chain_, pt); }
		virtual CPoint2d TransI(CPoint2d const& pt) const override { return ChainTransI(chain_, pt); }
		virtual CPoint3d Trans(CPoint3d const& pt) const override  { return ChainTrans(chain_, pt); }
		virtual CPoint3d TransI(CPoint3d const& pt) const override { return ChainTransI(chain_, pt); }
		virtual double Trans(double dLength) const override  { return ChainTrans(chain_, dLength); }
		virtual double TransI(double dLength) const override { return ChainTransI(chain_, dLength); }

		virtual bool IsRightHanded() const override {
			bool bRightHanded{true};
			for (auto const& ct : chain_) {
				if (!ct.IsRightHanded())
					bRightHanded = !bRightHanded;
			}
			return bRightHanded;
		}

	};


	//-----------------------------------------------------------------------------
	/// @brief class CCoordTrans2d 
	/// TARGET = scale * mat ( SOURCE - origin ) + offset
	class GTL_CLASS CCoordTrans2d : public ICoordTrans {
	public:
		using mat_t = cv::Matx22d;
	public:
		double scale_{1.0};	// additional scale value
		mat_t mat_;			// transform matrix
		CPoint2d origin_;	// pivot of source coordinate
		CPoint2d offset_;	// pivot of target coordinate

	public:
		using base_t = ICoordTrans;

		friend class boost::serialization::access;
		template < typename tBoostArchive >
		void serialize(tBoostArchive &ar, unsigned int const version) {
			ar & boost::serialization::base_object<base_t>(*this);
			ar & scale_;
			for (auto& v : mat_.val)
				ar & v;
			ar & origin_ & offset_;
		}

		GTL__VIRTUAL_DYNAMIC_DERIVED(CCoordTrans2d);

	public:
		// Constructors
		CCoordTrans2d(double scale = 1.0, mat_t const& m = mat_t::eye(), CPoint2d const& origin = {}, CPoint2d const& offset = {}) :
			scale_{scale},
			mat_(m),
			origin_(origin),
			offset_(offset)
		{
		}

		CCoordTrans2d(CCoordTrans2d const& B) = default;
		CCoordTrans2d& operator = (CCoordTrans2d const& B) = default;

		//bool operator == (const CCoordTrans2d& B) const { return (scale_ == B.scale_) && (mat_ == B.mat_) && (origin_ == B.origin_) && (offset_ == B.offset_); }
		//bool operator != (const CCoordTrans2d& B) const { return !(*this == B); }
		//-------------------------------------------------------------------------
		std::optional<CCoordTrans2d> GetInverse() const {
			// Scale
			double scale = 1/scale_;
			if (!std::isfinite(scale))
				return {};

			// Matrix
			bool bOK {};
			auto mat = mat_.inv(0, &bOK);
			if (!bOK)
				return {};

			return CCoordTrans2d(scale, mat, offset_, origin_);
		}

		//-------------------------------------------------------------------------
		// Setting
		void Set(double scale = 1., mat_t const& m = mat_t::eye(), CPoint2d const& origin = {}, CPoint2d const& offset = {}) {
			scale_ = scale;
			mat_ = m;
			origin_ = origin;
			offset_ = offset;
		}
		void Set(double scale, rad_t angle, CPoint2d const& origin = {}, CPoint2d const& offset = {}) {
			scale_ = scale;
			mat_ = GetRotatingMatrix(angle);
			origin_ = origin;
			offset_ = offset;
		}

		bool SetFrom2Points(std::span<CPoint2d const, 2> ptsSource, std::span<CPoint2d const, 2> ptsTarget, bool bCalcScale = true, double dMinDeterminant = 0.0, bool bRightHanded = true) {
			std::array<CPoint2d, 3> ptsS{ptsSource[0], ptsSource[1]}, ptsT{ptsTarget[0], ptsTarget[1]};
			// 세번째 점은 각각, 0번 기준으로 1번을 90도 회전시킴
			ptsS[2].x = -(ptsS[1].y-ptsS[0].y) + ptsS[0].x;
			ptsS[2].y =  (ptsS[1].x-ptsS[0].x) + ptsS[0].y;
			if (bRightHanded) {
				ptsT[2].x = -(ptsT[1].y-ptsT[0].y) + ptsT[0].x;
				ptsT[2].y =  (ptsT[1].x-ptsT[0].x) + ptsT[0].y;
			}
			else {
				ptsT[2].x =  (ptsT[1].y-ptsT[0].y) + ptsT[0].x;
				ptsT[2].y = -(ptsT[1].x-ptsT[0].x) + ptsT[0].y;
			}

			return SetFrom3Points(ptsS, ptsT, bCalcScale, dMinDeterminant);
		}
		bool SetFrom3Points(std::span<CPoint2d const, 3> ptsSource, std::span<CPoint2d const, 3> ptsTarget, bool bCalcScale = true, double dMinDeterminant = 0.0) {
			origin_ = ptsSource[0];
			offset_ = ptsTarget[0];
			auto v1s = ptsSource[1] - origin_;
			auto v2s = ptsSource[2] - origin_;
			auto v1t = ptsTarget[1] - offset_;
			auto v2t = ptsTarget[2] - offset_;
			double dLenSource = v1s.GetLength();
			double dLenTarget = v1t.GetLength();

			if ( (dLenSource == 0.0) || (dLenTarget == 0.0) )
				return false;

			cv::Matx22d matSource;
			matSource(0, 0) = v1s.x;
			matSource(1, 0) = v1s.y;
			matSource(0, 1) = v2s.x;
			matSource(1, 1) = v2s.y;

			// Check.
			double d = cv::determinant(matSource);
			if (fabs(d) <= dMinDeterminant)
				return false;

			cv::Matx22d matTarget;
			matTarget(0, 0) = v1t.x;
			matTarget(1, 0) = v1t.y;
			matTarget(0, 1) = v2t.x;
			matTarget(1, 1) = v2t.y;

			mat_ = matTarget * matSource.inv();

			scale_ = cv::determinant(mat_);
			mat_ /= scale_;

			if (!bCalcScale)
				scale_ = 1.0;

			return true;
		}

		static inline mat_t GetRotatingMatrix(rad_t angle) {
			double c{cos(angle)}, s{sin(angle)};
			return mat_t(c, -s, s, c);
		}
		void SetRotaionalMatrix(rad_t angle) {
			mat_ = GetRotatingMatrix(angle);
		}
		void RotateMatrix(rad_t angle) {
			mat_ = GetRotatingMatrix(angle) * mat_;
		}
		void Rotate(rad_t angle, CPoint2d const& ptCenter) {
			CCoordTrans2d backup(*this);
			Set(1.0, angle, ptCenter, ptCenter);
			*this *= backup;
		}
		void NegMX();
		void NegMY();
		void FlipMX();
		void FlipMY();
		void FlipX(double x = 0);
		void FlipY(double y = 0);
		void FlipXY(double x = 0, double y = 0);
		template < class TPOINT >
		void FlipXY(TPOINT const pt) { FlipXY(pt.x, pt.y); }

		//-------------------------------------------------------------------------
		// Operation
		//
		virtual CPoint2d Trans(CPoint2d const& pt) const {
			double x = pt.x - origin_.x;
			double y = pt.y - origin_.y;
			CPoint2d ptT;
			ptT = scale_ * (mat_ * (pt-origin_)) + offset_;
			return ptT;
		}
		virtual CPoint3d Trans(CPoint3d const& pt) const {
			CPoint2d ptT(Trans((CPoint2d&)pt));
			return CPoint3d(ptT.x, ptT.y, pt.z);
		}
		virtual CPoint2d TransI(CPoint2d const& pt) const {
			return GetInverse().value_or(CCoordTrans2d{}).Trans(pt);
		}
		virtual CPoint3d TransI(CPoint3d const& pt) const {
			CPoint2d ptT(GetInverse().Trans(pt));
			return CPoint3d(ptT.x, ptT.y, pt.z);
		}

		virtual double Trans(double dLength) const {
			CPoint2d pt0 = Trans(CPoint2d(0, 0));
			CPoint2d pt1 = Trans(CPoint2d(1, 1));
			double scale = pt0.Distance(pt1) / sqrt(2.0);
			return scale * dLength;
		}
		virtual double TransI(double dLength) const {
			CPoint2d pt0 = Trans(CPoint2d(0, 0));
			CPoint2d pt1 = Trans(CPoint2d(1, 1));
			double scale = sqrt(2.0) / pt0.Distance(pt1);
			return scale * dLength;
		}
		virtual bool IsRightHanded() const;

		CCoordTrans2d& operator *= (CCoordTrans2d const& B);
		CCoordTrans2d operator * (CCoordTrans2d const& B) const;


	};

#if 0
	//=============================================================================
	class AFX_EXT_CLASS_XMU CCoordTransNL : public CCoordTrans2d {
	public:
		double m_mat02 = 0, m_mat12 = 0;
	public:
		// Constructors
		CCoordTransNL(double scale = 1.0, const cv::Matx23d& m = cv::Matx23d::eye(), CPoint2d const& origin = CPoint2d(), CPoint2d const& offset = CPoint2d()) :
			CCoordTrans2d(scale, cv::Matx22d(m(0, 0), m(0, 1), m(1, 0), m(1, 1)), origin, offset)
		{
		}

		CCoordTransNL(CCoordTransNL const& B) = default;
		CCoordTransNL(CCoordTrans2d const& B) : CCoordTrans2d(B), m_mat02(0), m_mat12(0) {}

		DECLARE_NEWCLONE(CCoordTransNL);

		CCoordTransNL& operator = (CCoordTransNL const& B) = default;
		CCoordTransNL& operator = (CCoordTrans2d const& B) { (CCoordTrans2d&)*this = B; m_mat02 = 0; m_mat12 = 0; return *this; }

		//bool operator == (CCoordTransNL const& B) const { return ((CCoordTrans2d&)*this == (CCoordTrans2d&)B) && (m_mat02 == B.m_mat02) && (m_mat12 == B.m_mat12); }
		//bool operator != (CCoordTransNL const& B) const { return !(*this == B); }

		//-------------------------------------------------------------------------
	protected:
		bool GetInverse(CCoordTransNL& ctI) const {
			if ( (m_mat02 == 0.0) && (m_mat12 == 0.0) )
				return __super::GetInverse(ctI);
			throw std::exception("GetInverse() NOT Supported");
		}
		CCoordTransNL GetInverse() const {
			if ( (m_mat02 == 0.0) && (m_mat12 == 0.0) )
				return __super::GetInverse();
			throw std::exception("GetInverse() NOT Supported");
		}

	public:
		//-------------------------------------------------------------------------
		// Setting
		bool Set(double scale, const cv::Matx22d& m, CPoint2d const& origin, CPoint2d const& offset);
		bool Set(double scale, const cv::Matx23d& m, CPoint2d const& origin, CPoint2d const& offset);

		bool SetFrom2Points(CPoint2d const ptsSource[], CPoint2d const ptsTarget[], bool bCalcScale = true, double dMinDeterminant = 0.0) {
			m_mat02 = m_mat12 = 0;
			return __super::SetFrom2Points(ptsSource, ptsTarget, bCalcScale, dMinDeterminant);
		}
		bool SetFrom3Points(CPoint2d const ptsSource[], CPoint2d const ptsTarget[], bool bCalcScale = true, double dMinDeterminant = 0.0) {
			m_mat02 = m_mat12 = 0;
			return __super::SetFrom3Points(ptsSource, ptsTarget, bCalcScale, dMinDeterminant);
		}
		bool SetFrom4Points(CPoint2d const ptsSource[4], CPoint2d const ptsTarget[4], bool bCalcScale = true, double dMinDeterminant = 0.0);

		void SetMatrix(double m00, double m01, double m02, double m10, double m11, double m12) {
			mat_(0, 0) = m00; mat_(0, 1) = m01; m_mat02 = m02;
			mat_(1, 0) = m10; mat_(1, 1) = m11; m_mat12 = m12;
		}
		void SetMatrix(const cv::Matx23d& m) {
			mat_(0, 0) = m(0, 0); mat_(0, 1) = m(0, 1); m_mat02 = m(0, 2);
			mat_(1, 0) = m(1, 0); mat_(1, 1) = m(1, 1); m_mat12 = m(1, 2);
		}
		void GetMatrix(double& m00, double& m01, double& m02, double& m10, double& m11, double& m12) const {
			m00 = mat_(0, 0); m01 = mat_(0, 1); m02 = m_mat02;
			m10 = mat_(1, 0); m11 = mat_(1, 1); m12 = m_mat12;
		}
		void GetMatrixXY(double& m02, double& m12) const { m02 = m_mat02; m12 = m_mat12; }
		void SetMatrixXY(double m02, double m12) { m_mat02 = m02; m_mat12 = m12; }

		void NegMX();
		void NegMY();
		void FlipMX();
		void FlipMY();
		void FlipX(double x = 0);
		void FlipY(double y = 0);
		void FlipXY(double x = 0, double y = 0);
		void FlipXY(CPoint2d const& pt) { FlipXY(pt.x, pt.y); }

		//-------------------------------------------------------------------------
		// Operation
		//
		virtual CPoint2d Trans(CPoint2d const& pt) const {
			CPoint2d p = pt-origin_;
			CPoint2d ptT;
			ptT = scale_ * (mat_ * p + CPoint2d(m_mat02, m_mat12)*p.x*p.y) + offset_;
			return ptT;
		}
		virtual CPoint3d Trans(CPoint3d const& pt) const {
			CPoint2d ptT(Trans((CPoint2d&)pt));
			return CPoint3d(ptT.x, ptT.y, pt.z);
		}
		virtual CPoint2d TransI(CPoint2d const& pt) const {
			ASSERT(false);
			return GetInverse().Trans(pt);
		}
		virtual CPoint3d TransI(CPoint3d const& pt) const {
			ASSERT(false);
			CPoint2d ptT(GetInverse().Trans(pt));
			return CPoint3d(ptT.x, ptT.y, pt.z);
		}

		friend CCoordTransNL operator * (CCoordTrans2d const& A, CCoordTransNL const& B);

		//
	public:
		template <class Archive> Archive& StoreTo(Archive& ar) const {
			ar << CStringA("CoordTransNL");
			ar << scale_
				<< mat_(0, 0) << mat_(0, 1) << m_mat02 << mat_(1, 0) << mat_(1, 1) << m_mat12
				<< origin_
				<< offset_
				;
			return ar;
		}
		template <class Archive> Archive& LoadFrom(Archive& ar) {
			CStringA str;
			ar >> str;
			if ( (str == "CoordTrans1.0") || (str == "CoordTrans2d") ) {
				ar >> scale_
					>> mat_(0, 0) >> mat_(0, 1) >> mat_(1, 0) >> mat_(1, 1)
					>> origin_
					>> offset_
					;
				m_mat02 = m_mat12 = 0;
			} else if ( (str == "CoordTrans2.0") || (str == "CoordTransNL") ) {
				ar >> scale_
					>> mat_(0, 0) >> mat_(0, 1) >> m_mat02 >> mat_(1, 0) >> mat_(1, 1) >> m_mat12
					>> origin_
					>> offset_
					;
			}
			return ar;
		}
		template <class Archive>
		friend Archive& operator << (Archive& ar, CCoordTransNL const& B) {
			return B.StoreTo(ar);
		}
		template <class Archive>
		friend Archive& operator >> (Archive& ar, CCoordTransNL& B) {
			return B.LoadFrom(ar);
		}

		template <class CProfileSection> 
		bool SyncData(bool bStore, CProfileSection& section) {
			__super::SyncData(bStore, section);
			section.SyncItemValue(bStore, _T("m02"), m_mat02);
			section.SyncItemValue(bStore, _T("m12"), m_mat12);
			return true;
		}

	public:
		FEM_BEGIN_TBL_C(CCoordTransNL, CCoordTrans2d)
			FEM_ADD_MEMBER(m_mat02)
			FEM_ADD_MEMBER(m_mat12)
		FEM_END_TBL()
	};


	//=============================================================================

	class AFX_EXT_CLASS_XMU CCoordTrans3d : public ICoordTrans {
	public:
		typedef cv::Matx33d mat_t;
	public:
		double scale_;
		mat_t mat_;
		CPoint3d origin_;
		CPoint3d offset_;

	public:
		// Constructors
		CCoordTrans3d(
			double scale = 1.0,
			mat_t const& mat = mat_t::eye(),		// I (eigen matrix)
			CPoint3d const& origin = CPoint3d(0, 0, 0),
			CPoint3d const& offset = CPoint3d(0, 0, 0)
			)
		{
			scale_ = scale;
			mat_ = mat;
			origin_ = origin;
			offset_ = offset;
		}
		CCoordTrans3d(CCoordTrans3d const& B) = default;
		CCoordTrans3d& operator = (CCoordTrans3d const& B) = default;
		//{
		//	if (this != &B) {
		//		scale_		= B.scale_;
		//		mat_			= B.mat_;
		//		origin_		= B.origin_;
		//		offset_		= B.offset_;
		//	}
		//	return *this;
		//}

		DECLARE_NEWCLONE(CCoordTrans3d);

		//bool operator == (CCoordTrans3d const& B) const { return (scale_ == B.scale_) && (mat_ == B.mat_) && (origin_ == B.origin_) && (offset_ == B.offset_); }
		//bool operator != (CCoordTrans3d const& B) const { return !(*this == B); }

		bool IsSame(CCoordTrans3d const& B) const {
			return (*this)(CPoint3d(0, 0, 0)) == B.Trans(CPoint3d(0, 0, 0))
				&& (*this)(CPoint3d(1, 0, 0)) == B.Trans(CPoint3d(1, 0, 0))
				&& (*this)(CPoint3d(0, 1, 0)) == B.Trans(CPoint3d(0, 1, 0))
				&& (*this)(CPoint3d(0, 0, 1)) == B.Trans(CPoint3d(0, 0, 1))
				;
		}

		//static bool IsValid(const cv::Mat& mat);
		//bool IsValid() const { return IsValid(mat_); }

		//-------------------------------------------------------------------------
		bool GetInverse(CCoordTrans3d& ctI) const;
		CCoordTrans3d GetInverse() const;

		//-------------------------------------------------------------------------
		// Setting
		bool Set(double scale = 1.0,
			mat_t const& mat = mat_t::eye(),		// I (eigen matrix)
			CPoint3d const& origin = CPoint3d(0, 0, 0),
			CPoint3d const& offset = CPoint3d(0, 0, 0));
		bool SetRXAxis(double scale = 1.0,
			rad_t dT = 0.0_rad,
			CPoint3d const& origin = CPoint3d(0, 0, 0),
			CPoint3d const& offset = CPoint3d(0, 0, 0));
		bool SetRYAxis(double scale = 1.0,
			rad_t dT = 0.0_rad,
			CPoint3d const& origin = CPoint3d(0, 0, 0),
			CPoint3d const& offset = CPoint3d(0, 0, 0));
		bool SetRZAxis(double scale = 1.0,
			rad_t dT = 0.0_rad,
			CPoint3d const& origin = CPoint3d(0, 0, 0),
			CPoint3d const& offset = CPoint3d(0, 0, 0));
		bool SetRXYZ(double scale = 1.0,
			rad_t dTx = 0.0_rad, rad_t dTy = 0.0_rad, rad_t dTz = 0.0_rad,
			CPoint3d const& origin = CPoint3d(0, 0, 0),
			CPoint3d const& offset = CPoint3d(0, 0, 0));

		bool SetFrom4Points(CPoint3d const pts0[4], CPoint3d const pts1[4], bool bCalcScale = true, double dMinDeterminant = 0.0);
		bool SetFrom3Points(CPoint3d const pts0[3], CPoint3d const pts1[3], bool bCalcScale = true, double dMinDeterminant = 0.0);
		//bool SetFrom2Points(CPoint3d const pts0[2], CPoint3d const pts1[2], bool bCalcScale = true, double dMinDeterminant = 0.0);

		void SetScale(double scale) { scale_ = scale; }
		double GetScale() const { return scale_; }
		void SetMatrix(double m[9]) {
			mat_ = mat_t(m);
		}
		void SetMatrix(mat_t const& mat) { mat_ = mat; }
		void SetMatrix(double m00, double m01, double m02, double m10, double m11, double m12, double m20, double m21, double m22) {
			double m[] = { m00, m01, m02, m10, m11, m12, m20, m21, m22 };
			SetMatrix(m);
		}
		void SetMatrixRotateXAxis(rad_t dT) {
			double c = cos(dT), s = sin(dT);
			SetMatrix(1, 0, 0, 0, c, -s, 0, s, c);
		}
		void SetMatrixRotateYAxis(rad_t dT) {
			double c = cos(dT), s = sin(dT);
			SetMatrix(c, 0, s, 0, 1, 0, -s, 0, c);
		}
		void SetMatrixRotateZAxis(rad_t dT) {
			double c = cos(dT), s = sin(dT);
			SetMatrix(c, -s, 0, s, c, 0, 0, 0, 1);
		}
		void SetMatrixRotateXYZ(rad_t dTx, rad_t dTy, rad_t dTz) { mat_ = GetRMatX(dTx) * GetRMatY(dTy) * GetRMatZ(dTz); }
		void SetMatrixRotateXZY(rad_t dTx, rad_t dTy, rad_t dTz) { mat_ = GetRMatX(dTx) * GetRMatZ(dTz) * GetRMatY(dTy); }
		void SetMatrixRotateYXZ(rad_t dTx, rad_t dTy, rad_t dTz) { mat_ = GetRMatY(dTy) * GetRMatX(dTx) * GetRMatY(dTz); }
		void SetMatrixRotateYZX(rad_t dTx, rad_t dTy, rad_t dTz) { mat_ = GetRMatY(dTy) * GetRMatZ(dTz) * GetRMatX(dTx); }
		void SetMatrixRotateZXY(rad_t dTx, rad_t dTy, rad_t dTz) { mat_ = GetRMatZ(dTz) * GetRMatX(dTx) * GetRMatY(dTy); }
		void SetMatrixRotateZYX(rad_t dTx, rad_t dTy, rad_t dTz) { mat_ = GetRMatZ(dTz) * GetRMatY(dTy) * GetRMatX(dTx); }
		mat_t& GetMatrix() { return mat_; }
		mat_t const& GetMatrix() const { return mat_; }
		void GetMatrix(double m[mat_t::cols * mat_t::rows]) const {
			//ASSERT((mat_.cols >= 3) && (mat_.rows >= 3) && (mat_.depth() == CV_64F));
			//if ( (mat_.cols < 3) && (mat_.rows < 3) && (mat_.depth() != CV_64F) )
			//	return;
			m[0] = mat_(0, 0); m[1] = mat_(0, 1); m[2] = mat_(0, 2);
			m[3] = mat_(1, 0); m[4] = mat_(1, 1); m[5] = mat_(1, 2);
			m[6] = mat_(2, 0); m[7] = mat_(2, 1); m[8] = mat_(2, 2);
		}
		void GetMatrix(double& m00, double& m01, double& m02, double& m10, double& m11, double& m12, double& m20, double& m21, double& m22) const {
			//ASSERT((mat_.cols >= 3) && (mat_.rows >= 3) && (mat_.depth() == CV_64F));
			//if ( (mat_.cols < 3) && (mat_.rows < 3) && (mat_.depth() != CV_64F) )
			//	return;
			m00 = mat_(0, 0); m01 = mat_(0, 1); m02 = mat_(0, 2);
			m10 = mat_(1, 0); m11 = mat_(1, 1); m12 = mat_(1, 2);
			m20 = mat_(2, 0); m21 = mat_(2, 1); m22 = mat_(2, 2);
		}

		void SetShift(double dShiftX, double dShiftY, double dShiftZ) {
			origin_.SetPoint(dShiftX, dShiftY, dShiftZ);
		}
		template < class TPOINT >
		void SetShift(TPOINT const& origin) {
			origin_.SetPoint(origin.x, origin.y, origin.z);
		}
		void GetShift(double& dShiftX, double& dShiftY, double& dShiftZ) const {
			dShiftX = origin_.x; dShiftY = origin_.y; dShiftZ = origin_.z;
		}
		template < class TPOINT >
		void GetShift(TPOINT& origin) const {
			origin.x = origin_.x; origin.y = origin_.y; origin.z = origin_.z;
		}
		CPoint3d const& GetShift() const { return origin_; }
		CPoint3d& GetShift() { return origin_; }

		void SetOffset(double dOffsetX, double dOffsetY, double dOffsetZ) {
			offset_.SetPoint(dOffsetX, dOffsetY, dOffsetZ);
		}
		template < class TPOINT >
		void SetOffset(TPOINT const& offset) {
			offset_.SetPoint(offset.x, offset.y, offset.z);
		}
		void GetOffset(double& dOffsetX, double& dOffsetY, double& dOffsetZ) const {
			dOffsetX = offset_.x; dOffsetY = offset_.y; dOffsetZ = offset_.z;
		}
		template < class TPOINT >
		void GetOffset(TPOINT& offset) const {
			offset.x = offset_.x; offset.y = offset_.y; offset.z = offset_.z;
		}
		CPoint3d const& GetOffset() const { return offset_; }
		CPoint3d& GetOffset() { return offset_; }

		void RotateMX(rad_t dTx);
		void RotateMY(rad_t dTy);
		void RotateMZ(rad_t dTz);
		void FlipMX();
		void FlipMY();
		void FlipMZ();
		void FlipX(double x = 0);
		void FlipY(double y = 0);
		void FlipZ(double z = 0);
		void FlipXY(double x = 0, double y = 0);
		void FlipYZ(double y = 0, double z = 0);
		void FlipZX(double z = 0, double x = 0);
		void FlipXYZ(double x = 0, double y = 0, double z = 0);
		void FlipXYZ(CPoint3d const& ptCenter);

		//-------------------------------------------------------------------------
		// Operation
		//

			// ptTarget = (scale + alpha) * m * ( ptSource - origin ) + offset
		void Trans(double x0, double y0, double z0, double& x1, double& y1, double& z1) const;

		virtual CPoint2d Trans(CPoint2d const& pt) const { return Trans(CPoint3d(pt.x, pt.y, 0)); }
		virtual CPoint2d TransI(CPoint2d const& pt) const { return TransI(CPoint3d(pt.x, pt.y, 0)); }
		virtual CPoint3d Trans(CPoint3d const& pt) const {
			CPoint3d ptT;
			ptT = scale_ * (mat_ * (pt - origin_)) + offset_;
			return ptT;
		}
		virtual CPoint3d TransI(CPoint3d const& pt) const {
			return GetInverse().Trans(pt);
		}

		virtual double TransLength(double dLength) const {
			CPoint3d pt0 = Trans(CPoint3d(0, 0, 0));
			CPoint3d pt1 = Trans(CPoint3d(1, 1, 1));
			double scale = pt0.Distance(pt1) / sqrt(3.0);
			return scale * dLength;
		}
		virtual double TransLengthI(double dLength) const {
			CPoint3d pt0 = Trans(CPoint3d(0, 0, 0));
			CPoint3d pt1 = Trans(CPoint3d(1, 1, 1));
			double scale = sqrt(3.0) / pt0.Distance(pt1);
			return scale * dLength;
		}
		bool IsRightHanded() const;


		CPoint3d Project(CPoint3d const& pt, CPoint3d const& alpha = CPoint3d(0, 0, 0)) const;

		CCoordTrans3d& operator *= (CCoordTrans3d const& B);
		CCoordTrans3d operator * (CCoordTrans3d const& B) const;

	//
	public:
		template <class Archive> Archive& StoreTo(Archive& ar) const {
			ar << CStringA("CoordTrans3d1.1");
			ar << scale_;
			double m[9];
			GetMatrix(m);
			ar << m[0] << m[1] << m[2] << m[3] << m[4] << m[5] << m[6] << m[7] << m[8];
			ar << origin_;
			ar << offset_;
			return ar;
		}
		template <class Archive> Archive& LoadFrom(Archive& ar) {
			CStringA str;
			ar >> str;
			if (str != "CoordTrans3d1.1")
				return ar;
			ar >> scale_;
			double m[9];
			ar >> m[0] >> m[1] >> m[2] >> m[3] >> m[4] >> m[5] >> m[6] >> m[7] >> m[8];
			SetMatrix(m);
			ar >> origin_;
			ar >> offset_;
			return ar;
		}
		template <class Archive>
		friend Archive& operator << (Archive& ar, CCoordTrans3d const& B) {
			return B.StoreTo(ar);
		}
		template <class Archive>
		friend Archive& operator >> (Archive& ar, CCoordTrans3d& B) {
			return B.LoadFrom(ar);
		}

		template <class CProfileSection> 
		bool SyncData(bool bStore, CProfileSection& section) {
			section.SyncItemValue(bStore, _T("Scale"), scale_);
			double m[9] = {
				1, 0, 0,
				0, 1, 0,
				0, 0, 1
			};
			if (bStore)
				GetMatrix(m);
			section.SyncItemValue(bStore, _T("m00"), m[0]);
			section.SyncItemValue(bStore, _T("m01"), m[1]);
			section.SyncItemValue(bStore, _T("m02"), m[2]);
			section.SyncItemValue(bStore, _T("m10"), m[3]);
			section.SyncItemValue(bStore, _T("m11"), m[4]);
			section.SyncItemValue(bStore, _T("m12"), m[5]);
			section.SyncItemValue(bStore, _T("m20"), m[6]);
			section.SyncItemValue(bStore, _T("m21"), m[7]);
			section.SyncItemValue(bStore, _T("m22"), m[8]);
			if (!bStore)
				SetMatrix(m);
			section.SyncItemValue(bStore, _T("Shfit"), origin_);
			section.SyncItemValue(bStore, _T("Offset"), offset_);
			return true;
		}

	public:
		static mat_t GetRMatX(rad_t dT);
		static mat_t GetRMatY(rad_t dT);
		static mat_t GetRMatZ(rad_t dT);
		static bool MakeProjection(CCoordTrans3d& ct, CPoint3d const& vecPlaneNorm, CPoint3d const& ptPlaneBase = CPoint3d());
		static bool MakeProjectionOntoXY(CCoordTrans3d& ct, CPoint3d const& vecPlaneX, CPoint3d const& vecPlaneY, CPoint3d const& ptPlaneBase = CPoint3d());

	public:
		FEM_BEGIN_TBL(CCoordTrans3d)
			FEM_ADD_MEMBER(scale_)
			FEM_ADD_MEMBER(mat_)
			FEM_ADD_MEMBER(origin_)
			FEM_ADD_MEMBER(offset_)
		FEM_END_TBL()
	};



	template < class CIPVar >
	bool SyncIPVarCT(bool bStore, CIPVar& varCT, CCoordTrans2d& ct) {
		if (bStore) {
			varCT.SetChildItemT("scale",	ct.scale_);
			varCT.SetChildItemT("m00",		ct.mat_(0, 0));
			varCT.SetChildItemT("m01",		ct.mat_(0, 1));
			varCT.SetChildItemT("m10",		ct.mat_(1, 0));
			varCT.SetChildItemT("m11",		ct.mat_(1, 1));
			varCT.SetChildItemT("Shift",	ct.origin_);
			varCT.SetChildItemT("Offset",	ct.offset_);
		} else {
			varCT.GetChildItemT("scale",	ct.scale_);
			varCT.GetChildItemT("m00",		ct.mat_(0, 0));
			varCT.GetChildItemT("m01",		ct.mat_(0, 1));
			varCT.GetChildItemT("m10",		ct.mat_(1, 0));
			varCT.GetChildItemT("m11",		ct.mat_(1, 1));
			varCT.GetChildItemT("Shift",	ct.origin_);
			varCT.GetChildItemT("Offset",	ct.offset_);
		}
		return true;
	}

	template < class CIPVar >
	bool SyncIPVarCT(bool bStore, CIPVar& varCT, CCoordTransNL& ct) {
		SyncIPVarCT(bStore, varCT, (CCoordTrans2d&)ct);
		if (bStore) {
			varCT.SetChildItemT("m02",		ct.m_mat02);
			varCT.SetChildItemT("m12",		ct.m_mat12);
		} else {
			varCT.GetChildItemT("m02",		ct.m_mat02);
			varCT.GetChildItemT("m12",		ct.m_mat12);
		}
		return true;
	}

#endif
#pragma pack(pop)
}	// namespace gtl

#endif

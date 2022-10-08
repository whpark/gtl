#pragma once

//////////////////////////////////////////////////////////////////////
//
// coord_trans.h: coord trans
//
// PWH
// 2019.11.02. 전체 수정
// 2021.05.21. renewal
// 2021.07.05. new - naming convention
//
//////////////////////////////////////////////////////////////////////

#include "gtl/_lib_gtl.h"
#include "gtl/concepts.h"
#include "gtl/misc.h"
#include "gtl/unit.h"
#include "gtl/dynamic.h"

#include "boost/ptr_container/ptr_deque.hpp"
#include "boost/serialization/base_object.hpp"
#include "boost/serialization/serialization.hpp"
//#include "boost/serialization/export.hpp"

//#include "opencv2/opencv.hpp"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/core/matx.hpp"

#include "coord_size.h"
#include "coord_point.h"
#include "coord_rect.h"

#if 1

namespace gtl {
#pragma pack(push, 8)

	//-----------------------------------------------------------------------------
	// interface CoordTrans;
	class ICoordTrans {
	public:
		using this_t = ICoordTrans;
		using value_type = double;
		using point3_t = TPoint3<value_type>;
		using point2_t = TPoint2<value_type>;
		using size2_t = TSize2<value_type>;
		using size3_t = TSize3<value_type>;

		// Constructors
		ICoordTrans() { }
		virtual ~ICoordTrans() { }
		auto operator <=> (ICoordTrans const&) const = default;

		GTL__DYNAMIC_VIRTUAL_INTERFACE(ICoordTrans);

	public:
		//friend class boost::serialization::access;
		template < typename Archive >
		friend void serialize(Archive &ar, ICoordTrans& ct, unsigned int const version) {
			//ar & boost::serialization::base_object<base_t>(*this);
		}

	public:
		virtual std::unique_ptr<ICoordTrans> GetInverse() const = 0;


		/// @brief pt -> pt2
		/// @return 
		[[ nodiscard ]] point3_t operator () (point3_t const& pt) const {
			return Trans(pt);
		}

		/// @brief 
		/// @tparam tcoord3d : Not point3_t but one of 3d Coord.
		/// @param pt 
		/// @return 
		template < typename tcoord3d > requires (gtlc::tcoord3<tcoord3d, double> and !std::is_same_v<point3_t, tcoord3d>)
		[[ nodiscard ]] tcoord3d operator () (tcoord3d const& pt) const {
			auto v{pt};
			((point3_t&)v) = Trans((point3_t&)pt);
			return v;
		}

		/// @brief tcoord3d : not a Coord, buf has x, y, z
		template < typename tcoord3d > requires (gtlc::has__xyz<tcoord3d>) and (!gtlc::tcoord3<tcoord3d, double>)
		[[ nodiscard ]] tcoord3d operator () (tcoord3d const& pt) const {
			auto v{pt};
			auto r = Trans(point3_t(pt.x, pt.y, pt.z));
			v.x = RoundOrForward<decltype(v.x)>(r.x);
			v.y = RoundOrForward<decltype(v.y)>(r.y);
			v.z = RoundOrForward<decltype(v.z)>(r.z);
			return v;
		}


		/// @brief pt -> pt2
		/// @param pt 
		/// @return 
		[[ nodiscard ]] point2_t operator () (point2_t const& pt) const {
			return Trans(pt);
		}

		/// @brief pt -> pt2
		/// @tparam tcoord3d : Not point3_t but one of 3d Coord.
		/// @param pt 
		/// @return 
		template < typename tcoord2d > requires (gtlc::tcoord2<tcoord2d, double> and !std::is_same_v<point2_t, tcoord2d>)
		[[ nodiscard ]] tcoord2d operator () (tcoord2d const& pt) const {
			auto v{pt};
			((point2_t&)v) = Trans((point2_t&)pt);
			return v;
		}

		/// @brief tcoord2d : not a Coord, buf has x, y
		template < typename tcoord2d > requires (gtlc::has__xy<tcoord2d>) and (!gtlc::tcoord2<tcoord2d, double>)
		[[ nodiscard ]] tcoord2d operator () (tcoord2d const& pt) const {
			auto v{pt};
			auto r = Trans(point2_t(pt.x, pt.y));
			v.x = RoundOrForward<decltype(v.x)>(r.x);
			v.y = RoundOrForward<decltype(v.y)>(r.y);
			return v;
		}


		/// @brief double -> double
		/// @return 
		[[ nodiscard ]] double TransI(double length) const {
			return TransInverse(length).value_or(0.0);
		}

		/// @brief pt -> pt2
		/// @return 
		[[ nodiscard ]] point3_t TransI(point3_t const& pt) const {
			return TransInverse(pt).value_or(point3_t{});
		}

		/// @brief 
		/// @tparam tcoord3d : Not point3_t but one of 3d Coord.
		/// @param pt 
		/// @return 
		template < typename tcoord3d > requires (gtlc::tcoord3<tcoord3d, double> and !std::is_same_v<point3_t, tcoord3d>)
		[[ nodiscard ]] tcoord3d TransI(tcoord3d const& pt) const {
			auto v{pt};
			((point3_t&)v) = TransInverse((point3_t&)pt).value_or(point3_t{});
			return v;
		}

		/// @brief tcoord3d : not a Coord, buf has x, y, z
		template < typename tcoord3d > requires (gtlc::has__xyz<tcoord3d>) and (!gtlc::tcoord3<tcoord3d, double>)
		[[ nodiscard ]] tcoord3d TransI(tcoord3d const& pt) const {
			auto v{pt};
			auto r = TransInverse(point3_t(pt.x, pt.y, pt.z)).value_or(point3_t{});
			v.x = RoundOrForward<decltype(v.x)>(r.x);
			v.y = RoundOrForward<decltype(v.y)>(r.y);
			v.z = RoundOrForward<decltype(v.z)>(r.z);
			return v;
		}


		/// @brief pt -> pt2
		/// @param pt 
		/// @return 
		[[ nodiscard ]] point2_t TransI(point2_t const& pt) const {
			return TransInverse(pt).value_or(point2_t{});
		}

		/// @brief pt -> pt2
		/// @tparam tcoord3d : Not point3_t but one of 3d Coord.
		/// @param pt 
		/// @return 
		template < typename tcoord2d > requires (gtlc::tcoord2<tcoord2d, double> and !std::is_same_v<point2_t, tcoord2d>)
		[[ nodiscard ]] tcoord2d TransI(tcoord2d const& pt) const {
			auto v{pt};
			((point2_t&)v) = TransInverse((point2_t&)pt).value_or(point2_t{});
			return v;
		}

		/// @brief tcoord2d : not a Coord, buf has x, y
		template < typename tcoord2d > requires (gtlc::has__xy<tcoord2d>) and (!gtlc::tcoord2<tcoord2d, double>)
		[[ nodiscard ]] tcoord2d TransI(tcoord2d const& pt) const {
			auto v{pt};
			auto r = TransInverse(point2_t(pt.x, pt.y)).value_or(point2_t{});
			v.x = RoundOrForward<decltype(v.x)>(r.x);
			v.y = RoundOrForward<decltype(v.y)>(r.y);
			return v;
		}

		//-------------------------------------------------------------------------
		// Operation
		//
		virtual [[nodiscard]] point2_t Trans(point2_t const& pt) const = 0;
		virtual [[nodiscard]] point3_t Trans(point3_t const& pt) const = 0;
		virtual [[nodiscard]] double Trans(double dLength) const = 0;
		virtual [[nodiscard]] std::optional<point2_t> TransInverse(point2_t const& pt) const = 0;
		virtual [[nodiscard]] std::optional<point3_t> TransInverse(point3_t const& pt) const = 0;
		virtual [[nodiscard]] std::optional<double> TransInverse(double dLength) const = 0;

		virtual [[nodiscard]] bool IsRightHanded() const = 0;
	};


	//-----------------------------------------------------------------------------
//	template < std::floating_point T >
	class xCoordTransChain : public ICoordTrans {
	public:
		using base_t = ICoordTrans;

	protected:
		boost::ptr_deque<ICoordTrans> chain_;	// 마지막 Back() CT부터 Front() 까지 Transform() 적용.

	public:
		//friend class boost::serialization::access;
		template < typename Archive >
		friend void serialize(Archive &ar, xCoordTransChain& ct, unsigned int const version) {
			//ar & boost::serialization::base_object<base_t>(ct);
			ar & (base_t&)ct;
			ar & ct.chain_;
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xCoordTransChain);

	public:
		// Constructors
		xCoordTransChain() = default;
		//virtual ~TCoordTransChain() { }
		xCoordTransChain(xCoordTransChain const& B) = default;
		xCoordTransChain& operator = (xCoordTransChain const& B) = default;
		auto operator <=> (xCoordTransChain const&) const = default;

		xCoordTransChain& operator *= (xCoordTransChain const& B)	{
			for (auto const& ct : chain_) 
				chain_.push_back(std::move(ct.NewClone()));
			return *this;
		}
		xCoordTransChain& operator *= (ICoordTrans const& B) {
			chain_.push_back(std::move(B.NewClone()));
			return *this;
		}
		[[nodiscard]] xCoordTransChain operator * (xCoordTransChain const& B) const {
			xCoordTransChain newChain;
			for (auto const& ct : chain_) {
				newChain.chain_.push_back(std::move(ct.NewClone()));
			}
			for (auto const& ct : B.chain_) {
				newChain.chain_.push_back(std::move(ct.NewClone()));
			}
			return newChain;
		}

		[[nodiscard]] xCoordTransChain operator * (ICoordTrans const& B) const {
			xCoordTransChain newChain;
			for (auto const& ct : chain_) {
				newChain.chain_.push_back(std::move(ct.NewClone()));
			}
			newChain.chain_.push_back(std::move(B.NewClone()));
			return newChain;
		}

		friend [[nodiscard]] xCoordTransChain operator * (ICoordTrans const& A, xCoordTransChain const& B) {
			xCoordTransChain newChain;
			newChain.chain_.push_back(std::move(A.NewClone()));
			for (auto const& ct : B.chain_) {
				newChain.chain_.push_back(std::move(ct.NewClone()));
			}
			return newChain;
		}


		virtual std::unique_ptr<ICoordTrans> GetInverse() const override {
			auto inv = std::make_unique<xCoordTransChain>();
			for (auto iter = chain_.rbegin(); iter != chain_.rend(); iter++) {
				if (auto r = iter->GetInverse(); r) {
					inv->chain_.push_back(std::move(r));
				} else {
					return {};
				}
			}
			return std::move(inv);
		}
		bool GetInv(xCoordTransChain& ctI) const {
			ctI.chain_.clear();
			for (auto iter = chain_.rbegin(); iter != chain_.rend(); iter++) {
				if (auto r = iter->GetInverse(); r) {
					ctI.chain_.push_back(std::move(r));
				} else {
					return false;
				}
			}
			return true;
		}


		void clear() { chain_.clear(); }

		//-------------------------------------------------------------------------
		// Operation
		//
		template < typename tchain, typename tpoint >
		[[nodiscard]] tpoint ChainTrans(tchain const& chain, tpoint const& pt) const {
			tpoint ptT(pt);
			for (auto iter = chain.rbegin(); iter != chain.rend(); iter++)
				ptT = iter->Trans(ptT);
			return ptT;
		}
		template < typename tchain, typename tpoint >
		[[nodiscard]] tpoint ChainTransI(tchain const& chain, tpoint const& pt) const {
			tpoint ptT(pt);
			for (auto iter = chain.begin(); iter != chain.end(); iter++) {
				if (auto r = iter->TransInverse(ptT); r) {
					ptT = r.value();
				} else {
					return {};
				}
			}
			return ptT;
		}

		virtual [[nodiscard]] point2_t Trans(point2_t const& pt) const override  { return ChainTrans(chain_, pt); }
		virtual [[nodiscard]] point3_t Trans(point3_t const& pt) const override  { return ChainTrans(chain_, pt); }
		virtual [[nodiscard]] double Trans(double dLength) const override  { return ChainTrans(chain_, dLength); }
		virtual [[nodiscard]] std::optional<point2_t> TransInverse(point2_t const& pt) const override { return ChainTransI(chain_, pt); }
		virtual [[nodiscard]] std::optional<point3_t> TransInverse(point3_t const& pt) const override { return ChainTransI(chain_, pt); }
		virtual [[nodiscard]] std::optional<double> TransInverse(double dLength) const override { return ChainTransI(chain_, dLength); }

		virtual [[nodiscard]] bool IsRightHanded() const override {
			bool bRightHanded{true};
			for (auto const& ct : chain_) {
				if (!ct.IsRightHanded())
					bRightHanded = !bRightHanded;
			}
			return bRightHanded;
		}

	};


	//-----------------------------------------------------------------------------
	/// @brief class TCoordTransDim 
	/// TARGET = scale * mat ( SOURCE - origin ) + offset
	template < int dim = 2 >
	class TCoordTransDim : public ICoordTrans {
		static_assert( dim == 2 or dim == 3 );
	public:
		using this_t = TCoordTransDim;
		using base_t = ICoordTrans;
		using mat_t = cv::Matx<double, dim, dim>;
		using point_t = TPointT<double, dim>;

	public:
		double m_scale{1.0};	// additional scale value
		mat_t m_mat;			// transforming matrix
		point_t m_origin;		// pivot of source coordinate
		point_t m_offset;		// pivot of target coordinate

	public:

		//friend class boost::serialization::access;
		template < typename Archive >
		friend void serialize(Archive &ar, TCoordTransDim& ct, unsigned int const version) {
			ar & ct;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, this_t& B) {
			//ar & boost::serialization::base_object<base_t>(*this);
			ar & (base_t&)B;
			ar & B.m_scale;
			for (auto& v : B.m_mat.val)
				ar & v;
			ar & B.m_origin & B.m_offset;
			return ar;
		}
		template < typename JSON > friend void from_json(JSON const& j, this_t& B) {
			j["scale"] = B.m_scale;
			if constexpr (dim == 2) {
				j["mat00"] = B.m_mat(0, 0); j["mat01"] = B.m_mat(0, 1);
				j["mat10"] = B.m_mat(1, 0); j["mat10"] = B.m_mat(1, 1);
			} else {
				j["mat00"] = B.m_mat(0, 0); j["mat01"] = B.m_mat(0, 1); j["mat02"] = B.m_mat(0, 2);
				j["mat10"] = B.m_mat(1, 0); j["mat11"] = B.m_mat(1, 1); j["mat12"] = B.m_mat(1, 2);
				j["mat20"] = B.m_mat(2, 0); j["mat21"] = B.m_mat(2, 1); j["mat22"] = B.m_mat(2, 2);
			}
			j["origin"] = B.m_origin;
			j["offset"] = B.m_offset;
		}
		template < typename JSON > friend void to_json(JSON& j, this_t const& B) {
			B.m_scale = j["scale"];
			if constexpr (dim == 2) {
				B.m_mat(0, 0) = j["mat00"]; B.m_mat(0, 1) = j["mat01"];
				B.m_mat(1, 0) = j["mat10"]; B.m_mat(1, 1) = j["mat10"];
			} else {
				B.m_mat(0, 0) = j["mat00"]; B.m_mat(0, 1) = j["mat01"]; B.m_mat(0, 2) = j["mat02"];
				B.m_mat(1, 0) = j["mat10"]; B.m_mat(1, 1) = j["mat11"]; B.m_mat(1, 2) = j["mat12"];
				B.m_mat(2, 0) = j["mat20"]; B.m_mat(2, 1) = j["mat21"]; B.m_mat(2, 2) = j["mat22"];
			}
			B.m_origin = j["origin"];
			B.m_offset = j["offset"];
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(TCoordTransDim);

	public:
		// Constructors
		TCoordTransDim(double scale = 1.0, mat_t const& m = mat_t::eye(), point_t const& origin = {}, point_t const& offset = {}) :
			m_scale{scale},
			m_mat(m),
			m_origin(origin),
			m_offset(offset)
		{
		}

		TCoordTransDim(TCoordTransDim const& B) = default;
		TCoordTransDim& operator = (TCoordTransDim const& B) = default;

		auto operator <=> (TCoordTransDim const&) const = default;

		//bool operator == (const TCoordTransDim& B) const { return (m_scale == B.m_scale) && (m_mat == B.m_mat) && (m_origin == B.m_origin) && (m_offset == B.m_offset); }
		//bool operator != (const TCoordTransDim& B) const { return !(*this == B); }
		//-------------------------------------------------------------------------
		std::unique_ptr<ICoordTrans> GetInverse() const override {
			// Scale
			double scale = 1/m_scale;
			if (!std::isfinite(scale))
				return {};

			// Matrix
			bool bOK {};
			auto mat = m_mat.inv(0, &bOK);
			if (!bOK)
				return {};

			return std::make_unique<TCoordTransDim>(scale, mat, m_offset, m_origin);
		}
		bool GetInv(TCoordTransDim& ctI) const {
			// Scale
			double scale = 1/m_scale;
			if (!std::isfinite(scale))
				return false;

			// Matrix
			bool bOK {};
			auto mat = m_mat.inv(0, &bOK);
			if (!bOK)
				return false;

			ctI.Init(scale, mat, m_offset, m_origin);
			return true;
		}

		//-------------------------------------------------------------------------
		// Setting
		void Init(double scale = 1., mat_t const& m = mat_t::eye(), point_t const& origin = {}, point_t const& offset = {}) {
			m_scale = scale;
			m_mat = m;
			m_origin = origin;
			m_offset = offset;
		}
		void Init(double scale, rad_t angle, point_t const& origin = {}, point_t const& offset = {}) requires (dim == 2) {
			m_scale = scale;
			m_mat = GetRotatingMatrix(angle);
			m_origin = origin;
			m_offset = offset;
		}

		[[nodiscard]] bool SetFrom2Points(std::span<point_t const> ptsSource, std::span<point_t const> ptsTarget, bool bCalcScale = true, double dMinDeterminant = 0.0, bool bRightHanded = true) 
			requires (dim == 2)
		{
			if ( (ptsSource.size() < 2) or (ptsTarget.size() < 2) )
				return false;

			std::array<point_t, 3> ptsS{ptsSource[0], ptsSource[1]}, ptsT{ptsTarget[0], ptsTarget[1]};
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
		[[nodiscard]] bool SetFrom3Points(std::span<point_t const> ptsSource, std::span<point_t const> ptsTarget, bool bCalcScale = true, double dMinDeterminant = 0.0)
			requires (dim == 2)
		{
			if ( (ptsSource.size() < 3) or (ptsTarget.size() < 3) )
				return false;

			auto v1s = ptsSource[1] - ptsSource[0];
			auto v2s = ptsSource[2] - ptsSource[0];
			auto v1t = ptsTarget[1] - ptsTarget[0];
			auto v2t = ptsTarget[2] - ptsTarget[0];
			double dLenSource = v1s.GetLength();
			double dLenTarget = v1t.GetLength();

			mat_t matS { v1s.x, v2s.x, v1s.y, v2s.y };

			// Check.
			double d = cv::determinant(matS);
			if (fabs(d) <= dMinDeterminant)
				return false;

			mat_t matT { v1t.x, v2t.x, v1t.y, v2t.y };

			m_mat = matT * matS.inv();

			m_scale = fabs(cv::determinant(m_mat));
			m_mat /= m_scale;

			if (!bCalcScale)
				m_scale = 1.0;

			m_origin = ptsSource[0];
			m_offset = ptsTarget[0];

			return true;
		}
		[[nodiscard]] bool SetFrom3Points(std::span<point_t const> ptsSource, std::span<point_t const> ptsTarget, bool bCalcScale = true, double dMinDeterminant = 0.0)
			requires (dim == 3)
		{
			if ( (ptsSource.size() < 3) or (ptsTarget.size() < 3) )
				return false;

			std::array<point_t, 4> pts0{ptsSource[0], ptsSource[1], ptsSource[2]}, pts1{ptsTarget[0], ptsTarget[1], ptsTarget[2]};

			pts0[3] = (pts0[1] - pts0[0]) * (pts0[2]-pts0[0]) + pts0[0];
			pts1[3] = (pts1[1] - pts1[0]) * (pts1[2]-pts1[0]) + pts1[0];

			return SetFrom4Points(pts0, pts1, bCalcScale, dMinDeterminant);
		}
		[[nodiscard]] bool SetFrom4Points(std::span<point_t const> ptsSource, std::span<point_t const> ptsTarget, bool bCalcScale = true, double dMinDeterminant = 0.0)
			requires (dim == 3)
		{
			if ( (ptsSource.size() < 4) or (ptsTarget.size() < 4) )
				return false;

			auto& pts0 = ptsSource;
			auto& pts1 = ptsTarget;
			double dLenSource = pts0[0].Distance(pts0[1]);
			double dLenTarget = pts1[0].Distance(pts1[1]);

			if ( (dLenSource == 0.0) || (dLenTarget == 0.0) )
				return false;

			mat_t matS;
			for (int i = 0; i < matS.cols; i++) {
				matS(0, i) = pts0[i+1].x - pts0[0].x;
				matS(1, i) = pts0[i+1].y - pts0[0].y;
				matS(2, i) = pts0[i+1].z - pts0[0].z;
			}
			// Check.
			double d = cv::determinant(matS);
			if (fabs(d) <= dMinDeterminant)
				return false;

			mat_t matT;
			for (int i = 0; i < matT.cols; i++) {
				matT(0, i) = pts1[i+1].x - pts1[0].x;
				matT(1, i) = pts1[i+1].y - pts1[0].y;
				matT(2, i) = pts1[i+1].z - pts1[0].z;
			}

			m_mat = matT * matS.inv();

			m_scale = cv::determinant(m_mat);
			m_mat /= m_scale;

			if (!bCalcScale)
				m_scale = 1.0;

			m_origin = ptsSource[0];
			m_offset = ptsTarget[0];

			return true;
		}

		static inline [[nodiscard]] mat_t GetRotatingMatrix(rad_t angle) requires (dim == 2) {
			double c{cos(angle)}, s{sin(angle)};
			return mat_t(c, -s, s, c);
		}
		static inline [[nodiscard]] mat_t GetRotatingMatrixXY(rad_t angle) requires (dim >= 3) {
			double c{cos(angle)}, s{sin(angle)};
			return mat_t{c, -s, 0, s, c, 0, 0, 0, 1};
		}
		static inline [[nodiscard]] mat_t GetRotatingMatrixYZ(rad_t angle) requires (dim >= 3) {
			double c{cos(angle)}, s{sin(angle)};
			return mat_t{1, 0, 0, 0, c, -s, 0, s, c};
		}
		static inline [[nodiscard]] mat_t GetRotatingMatrixZX(rad_t angle) requires (dim >= 3) {
			double c{cos(angle)}, s{sin(angle)};
			return mat_t{c, 0, s, 0, 1, 0, -s, 0, c};
		}

		void SetMatrixRotaional(rad_t angle) requires (dim == 2) {
			m_mat = GetRotatingMatrix(angle);
		}
		void RotateMatrix(rad_t angle) requires (dim == 2) {
			m_mat = GetRotatingMatrix(angle) * m_mat;
		}
		void Rotate(rad_t angle, point_t const& ptCenter) requires (dim == 2) {
			auto backup(*this);
			Init(1.0, angle, ptCenter, ptCenter);
			*this *= backup;
		}
		void FlipSourceX()										{ for (int i {}; i < m_mat.rows; i++) m_mat(i, 0) = -m_mat(i, 0); }
		void FlipSourceY()										{ for (int i {}; i < m_mat.rows; i++) m_mat(i, 1) = -m_mat(i, 1); }
		void FlipSourceZ()					requires (dim >= 3) { for (int i {}; i < m_mat.rows; i++) m_mat(i, 2) = -m_mat(i, 2); }
		void FlipSourceX(double x)								{ FlipSourceX(); m_origin.x = 2*x - m_origin.x; }
		void FlipSourceY(double y)								{ FlipSourceY(); m_origin.y = 2*y - m_origin.y; }
		void FlipSourceZ(double z)			requires (dim >= 3) { FlipSourceY(); m_origin.z = 2*z - m_origin.z; }

		void FlipSource()										{ m_mat = -m_mat; }
		void FlipSource(point_t pt)								{ m_mat = -m_mat; m_origin = 2*pt - m_origin; }

		void FlipMatX()											{ for (int i {}; i < m_mat.cols; i++) m_mat(0, i) = -m_mat(0, i); }
		void FlipMatY()											{ for (int i {}; i < m_mat.cols; i++) m_mat(1, i) = -m_mat(1, i); }
		void FlipMatZ()						requires (dim >= 3) { for (int i {}; i < m_mat.cols; i++) m_mat(2, i) = -m_mat(2, i); }
		void FlipTargetX(double x = {})							{ FlipMatX(); m_offset.x = 2*x - m_offset.x; }
		void FlipTargetY(double y = {})							{ FlipMatY(); m_offset.y = 2*y - m_offset.y; }
		void FlipTargetZ(double z = {})		requires (dim >= 3) { FlipMatZ(); m_offset.z = 2*z - m_offset.z; }
		void FlipTarget(point_t pt = {})						{ m_mat = -m_mat; m_offset = 2*pt - m_offset; }

		//-------------------------------------------------------------------------
		// Operation
		//
		virtual [[nodiscard]] point2_t Trans(point2_t const& pt) const override {
			if constexpr (dim == 2) {
				return m_scale * (m_mat * (pt-m_origin)) + m_offset;
			} else if constexpr (dim == 3) {
				return m_scale * (m_mat * (point_t(pt)-m_origin)) + m_offset;
			}
		}
		virtual [[nodiscard]] point3_t Trans(point3_t const& pt) const override {
			if constexpr (dim == 2) {
				point3_t ptNew(Trans((point2_t&)pt));
				ptNew.z = pt.z;
				return ptNew;
			} else if constexpr (dim == 3) {
				return m_scale * (m_mat * (pt-m_origin)) + m_offset;
			}
		}
		virtual [[nodiscard]] std::optional<point2_t> TransInverse(point2_t const& pt) const override {
			if (auto r = GetInverse(); r)
				return (*r)(pt);
			return {};
		}
		virtual [[nodiscard]] std::optional<point3_t> TransInverse(point3_t const& pt) const override {
			if (auto r = GetInverse(); r)
				return (*r)(pt);
			return {};
		}

		virtual [[nodiscard]] double Trans(double dLength) const override {
			point_t pt0 = Trans(point_t::All(0));
			point_t pt1 = Trans(point_t::All(1));
			double scale = pt0.Distance(pt1) / sqrt((double)dim);
			return scale * dLength;
		}
		virtual [[nodiscard]] std::optional<double> TransInverse(double dLength) const override {
			point2_t pt0 = Trans(point_t::All(0));
			point2_t pt1 = Trans(point_t::All(1));
			double scale = sqrt((double)dim) / pt0.Distance(pt1);
			return scale * dLength;
		}
		virtual [[nodiscard]] bool IsRightHanded() const override {
			return cv::determinant(m_mat) >= 0;
		}

		TCoordTransDim& operator *= (TCoordTransDim const& B) {
			// 순서 바꾸면 안됨.
			m_offset		= m_scale * m_mat * (B.m_offset - m_origin) + m_offset;

			m_origin		= B.m_origin;
			m_mat		= m_mat * B.m_mat;
			m_scale		= m_scale * B.m_scale;
			return *this;
		}
		[[nodiscard]] TCoordTransDim operator * (TCoordTransDim const& B) const {
			auto C(*this);
			return C *= B;
		}

	};


	//template GTL__CLASS TCoordTransChain<double>;
	//using xCoordTransChain = TCoordTransChain<double>;

	//template TCoordTransDim<2>;
	using xCoordTrans2d = TCoordTransDim<2>;

	//template TCoordTransDim<3>;
	using xCoordTrans3d = TCoordTransDim<3>;


	//GTL__CLASS xCoordTrans2d;
	//GTL__CLASS xCoordTrans3d;

#pragma pack(pop)
}	// namespace gtl

#endif

module;

//////////////////////////////////////////////////////////////////////
//
// coord_trans.h: coord trans
//
// PWH
// 2019.11.02. 전체 수정
// 2021.05.21. renewal
//
//////////////////////////////////////////////////////////////////////

#include <cmath>
#include <memory>
#include <deque>
#include <span>
#include <optional>

#include "gtl/_config.h"
#include "gtl/_macro.h"

//#include "boost/ptr_container/ptr_container.hpp"
//#include "boost/serialization/base_object.hpp"
//#include "boost/archive/polymorphic_xml_iarchive.hpp"
//#include "boost/archive/polymorphic_xml_oarchive.hpp"

//#include "opencv2/opencv.hpp"
//#include "opencv2/opencv_modules.hpp"
//#include "opencv2/core.hpp"

export module gtl:coord_trans;
import :concepts;
import :coord_point;
import :coord_size;
//import :matrix;

export namespace gtl {

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
		point3_t operator () (point3_t const& pt) const {
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

		//-------------------------------------------------------------------------
		// Operation
		//
		virtual [[nodiscard]] point2_t Trans(point2_t const& pt) const = 0;
		virtual [[nodiscard]] point3_t Trans(point3_t const& pt) const = 0;
		virtual [[nodiscard]] double Trans(double dLength) const = 0;
		virtual [[nodiscard]] std::optional<point2_t> TransI(point2_t const& pt) const = 0;
		virtual [[nodiscard]] std::optional<point3_t> TransI(point3_t const& pt) const = 0;
		virtual [[nodiscard]] std::optional<double> TransI(double dLength) const = 0;

		virtual [[nodiscard]] bool IsRightHanded() const = 0;
	};

#if 0
	//-----------------------------------------------------------------------------
//	template < std::floating_point T >
	class CCoordTransChain : public ICoordTrans {
	public:
		using base_t = ICoordTrans;

	protected:
		boost::ptr_deque<ICoordTrans> chain_;	// 마지막 Back() CT부터 Front() 까지 Transform() 적용.

	public:
		//friend class boost::serialization::access;
		template < typename Archive >
		friend void serialize(Archive &ar, CCoordTransChain& ct, unsigned int const version) {
			//ar & boost::serialization::base_object<base_t>(ct);
			ar & (base_t&)ct;
			ar & ct.chain_;
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(CCoordTransChain);

	public:
		// Constructors
		CCoordTransChain() = default;
		//virtual ~TCoordTransChain() { }
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
		[[nodiscard]] CCoordTransChain operator * (CCoordTransChain const& B) const {
			CCoordTransChain newChain;
			for (auto const& ct : chain_) {
				newChain.chain_.push_back(std::move(ct.NewClone()));
			}
			for (auto const& ct : B.chain_) {
				newChain.chain_.push_back(std::move(ct.NewClone()));
			}
			return newChain;
		}

		[[nodiscard]] CCoordTransChain operator * (ICoordTrans const& B) const {
			CCoordTransChain newChain;
			for (auto const& ct : chain_) {
				newChain.chain_.push_back(std::move(ct.NewClone()));
			}
			newChain.chain_.push_back(std::move(B.NewClone()));
			return newChain;
		}

		friend [[nodiscard]] CCoordTransChain operator * (ICoordTrans const& A, CCoordTransChain const& B) {
			CCoordTransChain newChain;
			newChain.chain_.push_back(std::move(A.NewClone()));
			for (auto const& ct : B.chain_) {
				newChain.chain_.push_back(std::move(ct.NewClone()));
			}
			return newChain;
		}

		virtual std::unique_ptr<ICoordTrans> GetInverse() const override {
			auto inv = std::make_unique<CCoordTransChain>();
			for (auto iter = chain_.rbegin(); iter != chain_.rend(); iter++) {
				if (auto r = iter->GetInverse(); r) {
					inv->chain_.push_back(std::move(r));
				} else {
					return {};
				}
			}
			return std::move(inv);
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
				if (auto r = iter->TransI(ptT); r) {
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
		virtual [[nodiscard]] std::optional<point2_t> TransI(point2_t const& pt) const override { return ChainTransI(chain_, pt); }
		virtual [[nodiscard]] std::optional<point3_t> TransI(point3_t const& pt) const override { return ChainTransI(chain_, pt); }
		virtual [[nodiscard]] std::optional<double> TransI(double dLength) const override { return ChainTransI(chain_, dLength); }

		virtual [[nodiscard]] bool IsRightHanded() const override {
			bool bRightHanded{true};
			for (auto const& ct : chain_) {
				if (!ct.IsRightHanded())
					bRightHanded = !bRightHanded;
			}
			return bRightHanded;
		}

	};
#endif

#if 0
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
		//using mat_t = TMatrix<double, dim, dim>;
		using point_t = TPointT<double, dim>;

	public:
		double scale_{1.0};	// additional scale value
		mat_t mat_;			// transforming matrix
		point_t origin_;	// pivot of source coordinate
		point_t offset_;	// pivot of target coordinate

	public:
		//friend class boost::serialization::access;
		template < typename Archive >
		friend void serialize(Archive &ar, TCoordTransDim& ct, unsigned int const version) {
			ar & ct;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, this_t& B) {
			//ar & boost::serialization::base_object<base_t>(*this);
			ar & (base_t&)B;
			ar & B.scale_;
			for (auto& v : B.mat_.val)
				ar & v;
			ar & B.origin_ & B.offset_;
			return ar;
		}
		template < typename JSON > friend void from_json(JSON const& j, this_t& B) {
			j["scale"] = B.scale_;
			if constexpr (dim == 2) {
				j["mat00"] = B.mat_(0, 0); j["mat01"] = B.mat_(0, 1);
				j["mat10"] = B.mat_(1, 0); j["mat10"] = B.mat_(1, 1);
			} else {
				j["mat00"] = B.mat_(0, 0); j["mat01"] = B.mat_(0, 1); j["mat02"] = B.mat_(0, 2);
				j["mat10"] = B.mat_(1, 0); j["mat11"] = B.mat_(1, 1); j["mat12"] = B.mat_(1, 2);
				j["mat20"] = B.mat_(2, 0); j["mat21"] = B.mat_(2, 1); j["mat22"] = B.mat_(2, 2);
			}
			j["origin"] = B.origin_;
			j["offset"] = B.offset_;
		}
		template < typename JSON > friend void to_json(JSON& j, this_t const& B) {
			B.scale_ = j["scale"];
			if constexpr (dim == 2) {
				B.mat_(0, 0) = j["mat00"]; B.mat_(0, 1) = j["mat01"];
				B.mat_(1, 0) = j["mat10"]; B.mat_(1, 1) = j["mat10"];
			} else {
				B.mat_(0, 0) = j["mat00"]; B.mat_(0, 1) = j["mat01"]; B.mat_(0, 2) = j["mat02"];
				B.mat_(1, 0) = j["mat10"]; B.mat_(1, 1) = j["mat11"]; B.mat_(1, 2) = j["mat12"];
				B.mat_(2, 0) = j["mat20"]; B.mat_(2, 1) = j["mat21"]; B.mat_(2, 2) = j["mat22"];
			}
			B.origin_ = j["origin"];
			B.offset_ = j["offset"];
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(TCoordTransDim);

	public:
		// Constructors
		TCoordTransDim(double scale = 1.0, mat_t const& m = mat_t::eye(), point_t const& origin = {}, point_t const& offset = {}) :
			scale_{scale},
			mat_(m),
			origin_(origin),
			offset_(offset)
		{
		}

		TCoordTransDim(TCoordTransDim const& B) = default;
		TCoordTransDim& operator = (TCoordTransDim const& B) = default;

		//bool operator == (const TCoordTransDim& B) const { return (scale_ == B.scale_) && (mat_ == B.mat_) && (origin_ == B.origin_) && (offset_ == B.offset_); }
		//bool operator != (const TCoordTransDim& B) const { return !(*this == B); }
		//-------------------------------------------------------------------------
		std::unique_ptr<ICoordTrans> GetInverse() const override {
			// Scale
			double scale = 1/scale_;
			if (!std::isfinite(scale))
				return {};

			// Matrix
			bool bOK {};
			auto mat = mat_.inv(0, &bOK);
			if (!bOK)
				return {};

			return std::make_unique<TCoordTransDim>(scale, mat, offset_, origin_);
		}

		//-------------------------------------------------------------------------
		// Setting
		void Init(double scale = 1., mat_t const& m = mat_t::eye(), point_t const& origin = {}, point_t const& offset = {}) {
			scale_ = scale;
			mat_ = m;
			origin_ = origin;
			offset_ = offset;
		}
		void Init(double scale, rad_t angle, point_t const& origin = {}, point_t const& offset = {}) requires (dim == 2) {
			scale_ = scale;
			mat_ = GetRotatingMatrix(angle);
			origin_ = origin;
			offset_ = offset;
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
			//double d = matS.determinant();
			if (fabs(d) <= dMinDeterminant)
				return false;

			mat_t matT { v1t.x, v2t.x, v1t.y, v2t.y };

			mat_ = matT * matS.inv();

			scale_ = fabs(cv::determinant(mat_));
			//scale_ = fabs(mat_.determinant());
			mat_ /= scale_;

			if (!bCalcScale)
				scale_ = 1.0;

			origin_ = ptsSource[0];
			offset_ = ptsTarget[0];

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
			//double d = matS.determinant();
			if (fabs(d) <= dMinDeterminant)
				return false;

			mat_t matT;
			for (int i = 0; i < matT.cols; i++) {
				matT(0, i) = pts1[i+1].x - pts1[0].x;
				matT(1, i) = pts1[i+1].y - pts1[0].y;
				matT(2, i) = pts1[i+1].z - pts1[0].z;
			}

			mat_ = matT * matS.inv();

			scale_ = cv::determinant(mat_);
			//scale_ = mat_.determinant();
			mat_ /= scale_;

			if (!bCalcScale)
				scale_ = 1.0;

			origin_ = ptsSource[0];
			offset_ = ptsTarget[0];

			return true;
		}

		static inline [[nodiscard]] mat_t GetRotatingMatrix(rad_t angle) requires (dim == 2) {
			double c{cos(angle)}, s{sin(angle)};
			return mat_t{c, -s, s, c};
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
			mat_ = GetRotatingMatrix(angle);
		}
		void RotateMatrix(rad_t angle) requires (dim == 2) {
			mat_ = GetRotatingMatrix(angle) * mat_;
		}
		void Rotate(rad_t angle, point_t const& ptCenter) requires (dim == 2) {
			auto backup(*this);
			Init(1.0, angle, ptCenter, ptCenter);
			*this *= backup;
		}
		void FlipSourceX()										{ for (int i {}; i < mat_.rows; i++) mat_(i, 0) = -mat_(i, 0); }
		void FlipSourceY()										{ for (int i {}; i < mat_.rows; i++) mat_(i, 1) = -mat_(i, 1); }
		void FlipSourceZ()					requires (dim >= 3) { for (int i {}; i < mat_.rows; i++) mat_(i, 2) = -mat_(i, 2); }
		void FlipSourceX(double x)								{ FlipSourceX(); origin_.x = 2*x - origin_.x; }
		void FlipSourceY(double y)								{ FlipSourceY(); origin_.y = 2*y - origin_.y; }
		void FlipSourceZ(double z)			requires (dim >= 3) { FlipSourceY(); origin_.z = 2*z - origin_.z; }

		void FlipSource()										{ mat_ = -mat_; }
		void FlipSource(point_t pt)								{ mat_ = -mat_; origin_ = 2*pt - origin_; }

		void FlipMatX()											{ for (int i {}; i < mat_.cols; i++) mat_(0, i) = -mat_(0, i); }
		void FlipMatY()											{ for (int i {}; i < mat_.cols; i++) mat_(1, i) = -mat_(1, i); }
		void FlipMatZ()						requires (dim >= 3) { for (int i {}; i < mat_.cols; i++) mat_(2, i) = -mat_(2, i); }
		void FlipTargetX(double x = {})							{ FlipMatX(); offset_.x = 2*x - offset_.x; }
		void FlipTargetY(double y = {})							{ FlipMatY(); offset_.y = 2*y - offset_.y; }
		void FlipTargetZ(double z = {})		requires (dim >= 3) { FlipMatZ(); offset_.z = 2*z - offset_.z; }
		void FlipTarget(point_t pt = {})						{ mat_ = -mat_; offset_ = 2*pt - offset_; }

		//-------------------------------------------------------------------------
		// Operation
		//
		virtual [[nodiscard]] point2_t Trans(point2_t const& pt) const override {
			if constexpr (dim == 2) {
				return scale_ * (mat_ * (pt-origin_)) + offset_;
			} else if constexpr (dim == 3) {
				return scale_ * (mat_ * (point_t(pt)-origin_)) + offset_;
			}
		}
		virtual [[nodiscard]] point3_t Trans(point3_t const& pt) const override {
			if constexpr (dim == 2) {
				point3_t ptNew(Trans((point2_t&)pt));
				ptNew.z = pt.z;
				return ptNew;
			} else if constexpr (dim == 3) {
				return scale_ * (mat_ * (pt-origin_)) + offset_;
			}
		}
		virtual [[nodiscard]] std::optional<point2_t> TransI(point2_t const& pt) const override {
			if (auto r = GetInverse(); r)
				return (*r)(pt);
			return {};
		}
		virtual [[nodiscard]] std::optional<point3_t> TransI(point3_t const& pt) const override {
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
		virtual [[nodiscard]] std::optional<double> TransI(double dLength) const override {
			point2_t pt0 = Trans(point_t::All(0));
			point2_t pt1 = Trans(point_t::All(1));
			double scale = sqrt((double)dim) / pt0.Distance(pt1);
			return scale * dLength;
		}
		virtual [[nodiscard]] bool IsRightHanded() const override {
			return cv::determinant(mat_) >= 0;
			//return mat_.determinant() >= 0;
		}

		TCoordTransDim& operator *= (TCoordTransDim const& B) {
			// 순서 바꾸면 안됨.
			//offset_		= scale_ * mat_ * (B.offset_ - origin_) + offset_;
			auto t = mat_*(B.offset_-origin_);
			offset_		= scale_ * mat_ * (B.offset_ - origin_) + offset_;

			origin_		= B.origin_;
			mat_		= mat_ * B.mat_;
			scale_		= scale_ * B.scale_;
			return *this;
		}
		[[nodiscard]] TCoordTransDim operator * (TCoordTransDim const& B) const {
			auto C(*this);
			return C *= B;
		}

	};


	//template GTL_CLASS TCoordTransChain<double>;
	//using CCoordTransChain = TCoordTransChain<double>;

	template TCoordTransDim<2>;
	using CCoordTrans2d = TCoordTransDim<2>;

	template TCoordTransDim<3>;
	using CCoordTrans3d = TCoordTransDim<3>;

#endif

	//GTL_CLASS CCoordTrans2d;
	//GTL_CLASS CCoordTrans3d;

}	// namespace gtl

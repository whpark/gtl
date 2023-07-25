#pragma once

//////////////////////////////////////////////////////////////////////
//
// coord_trans.h: coord trans
//
// PWH
// 2023.07.24. using opencv 4x4 matrix
//
//////////////////////////////////////////////////////////////////////

#include "coord_trans.h"

namespace gtl {
#pragma pack(push, 8)

	//-----------------------------------------------------------------------------
	class xCoordTransP33 : public ICoordTrans {
	public:
		using this_t = xCoordTransP33;
		using base_t = ICoordTrans;
		using mat_t = cv::Matx33d;
		using point_t = ICoordTrans::point2_t;

	protected:
		mat_t m_mat {mat_t::eye()};

	public:
		GTL__DYNAMIC_VIRTUAL_DERIVED(xCoordTransP33);

	public:
		// Constructors
		xCoordTransP33() = default;
		xCoordTransP33(xCoordTransP33 const& B) = default;
		explicit xCoordTransP33(mat_t const& mat) : m_mat(mat) {};
		xCoordTransP33& operator = (xCoordTransP33 const& B) = default;
		auto operator <=> (xCoordTransP33 const&) const = default;

		xCoordTransP33& operator *= (xCoordTransP33 const& B)	{
			m_mat = m_mat * B.m_mat;
			return *this;
		}
		//xCoordTransP33& operator *= (ICoordTrans const& B) {
		//	chain_.push_back(std::move(B.NewClone()));
		//	return *this;
		//}

		[[nodiscard]] xCoordTransP33 operator * (xCoordTransP33 const& B) const {
			xCoordTransP33 newChain;
			newChain.m_mat = m_mat * B.m_mat;
			return newChain;
		}

		//[[nodiscard]] xCoordTransP33 operator * (ICoordTrans const& B) const {
		//	xCoordTransP33 newChain;
		//	for (auto const& ct : chain_) {
		//		newChain.chain_.push_back(std::move(ct.NewClone()));
		//	}
		//	newChain.chain_.push_back(std::move(B.NewClone()));
		//	return newChain;
		//}

		//friend [[nodiscard]] xCoordTransP33 operator * (ICoordTrans const& A, xCoordTransP33 const& B) {
		//	xCoordTransP33 newChain;
		//	newChain.chain_.push_back(std::move(A.NewClone()));
		//	for (auto const& ct : B.chain_) {
		//		newChain.chain_.push_back(std::move(ct.NewClone()));
		//	}
		//	return newChain;
		//}

		virtual std::unique_ptr<ICoordTrans> GetInverse() const override {
			bool bOK{};
			auto mat = m_mat.inv(cv::DECOMP_LU, &bOK);
			if (!bOK)
				return {};
			return std::make_unique<this_t>(mat);
		}

		bool GetInv(xCoordTransP33& ctI) const {
			bool bOK{};
			auto mat = m_mat.inv(cv::DECOMP_LU, &bOK);
			ctI.m_mat = bOK ? mat : mat_t::eye();
			return bOK;
		}

		void clear() {
			m_mat = m_mat.eye();
		}

		[[nodiscard]] bool SetFrom4Points(std::span<point_t const> ptsSource, std::span<point_t const> ptsTarget) {
			if ( (ptsSource.size() < 4) or (ptsTarget.size() < 4) )
				return false;

			double dLenSource = ptsSource[0].Distance(ptsSource[1]);
			double dLenTarget = ptsTarget[0].Distance(ptsTarget[1]);
			if ( (dLenSource == 0.0) || (dLenTarget == 0.0) )
				return false;

			cv::Point2f pts0[4], pts1[4];
			for (int i = 0; i < 4; i++) {
				pts0[i] = ptsSource[i];
				pts1[i] = ptsTarget[i];
			}

			cv::Mat m = cv::getPerspectiveTransform(pts0, pts1, cv::DECOMP_LU);
			m_mat = m;

			return true;
		}

		//-------------------------------------------------------------------------
		// Operation
		//
		virtual [[nodiscard]] point2_t Trans(point2_t const& pt) const override {
			point3_t ptNew;
			ptNew.x = m_mat(0, 0) * pt.x + m_mat(0, 1) * pt.y + m_mat(0, 2);
			ptNew.y = m_mat(1, 0) * pt.x + m_mat(1, 1) * pt.y + m_mat(1, 2);
			//ptNew.z = m_mat(2, 0) * pt.x + m_mat(2, 1) * pt.y + m_mat(2, 2);
			double d = m_mat(2, 0) * pt.x + m_mat(2, 1) * pt.y + m_mat(2, 2);
			//double d = m_mat(3, 0) * pt.x + m_mat(3, 1) * pt.y + m_mat(3, 2);
			if ( (d != 0.0) and (d != 1.0)/* and std::isfinite(d) */) {
				ptNew.x /= d;
				ptNew.y /= d;
			}
			return ptNew;
		}
		virtual [[nodiscard]] point3_t Trans(point3_t const& pt) const override  {
			return Trans(point2_t{pt.x, pt.y});
		}
		virtual [[nodiscard]] double Trans(double dLength) const override  {
			return std::sqrt(cv::determinant(m_mat))*dLength;
		}
		virtual [[nodiscard]] std::optional<point2_t> TransInverse(point2_t const& pt) const override {
			this_t ctI;
			if (!GetInv(ctI))
				return {};
			return ctI.Trans(pt);
		}
		virtual [[nodiscard]] std::optional<point3_t> TransInverse(point3_t const& pt) const override {
			if (pt.z == 0.0)
				return TransInverse(point2_t{pt.x, pt.y});
			return {};
		}
		virtual [[nodiscard]] std::optional<double> TransInverse(double dLength) const override {
			double d = cv::determinant(m_mat);
			if (d == 0)
				return {};
			return dLength / std::sqrt(d);
		}

		virtual [[nodiscard]] bool IsRightHanded() const override {
			bool bRightHanded{cv::determinant(m_mat) > 0};
			return bRightHanded;
		}

		static inline [[nodiscard]] mat_t GetRotatingMatrixXY(rad_t angle) {
			double c{cos(angle)}, s{sin(angle)};
			//return mat_t{c, -s, 0., 0., /**/ s, c, 0., 0., /**/ 0., 0., 1., 0., /**/ 0., 0., 0., 1. };
		}
		static inline [[nodiscard]] mat_t GetRotatingMatrixYZ(rad_t angle) {
			double c{cos(angle)}, s{sin(angle)};
			//return mat_t{1., 0., 0., 0., /**/ 0., c, -s, 0., /**/ 0., s, c, 0., /**/ 0., 0., 0., 1. };
		}
		static inline [[nodiscard]] mat_t GetRotatingMatrixZX(rad_t angle) {
			double c{cos(angle)}, s{sin(angle)};
			//return mat_t{c, 0., s, 0., /**/ 0., 1., 0., 0., /**/ -s, 0., c, 0., /**/ 0., 0., 0., 1., };
		}

	};

	//-----------------------------------------------------------------------------
	class xCoordTransP44 : public ICoordTrans {
	public:
		using this_t = xCoordTransP44;
		using base_t = ICoordTrans;
		using mat_t = cv::Matx44d;

	protected:
		mat_t m_mat {mat_t::eye()};

	public:
		GTL__DYNAMIC_VIRTUAL_DERIVED(xCoordTransP44);

	public:
		// Constructors
		xCoordTransP44() = default;
		xCoordTransP44(xCoordTransP44 const& B) = default;
		explicit xCoordTransP44(mat_t const& mat) : m_mat(mat) {};
		xCoordTransP44& operator = (xCoordTransP44 const& B) = default;
		auto operator <=> (xCoordTransP44 const&) const = default;

		xCoordTransP44& operator *= (xCoordTransP44 const& B)	{
			m_mat = m_mat * B.m_mat;
			return *this;
		}
		//xCoordTransP44& operator *= (ICoordTrans const& B) {
		//	chain_.push_back(std::move(B.NewClone()));
		//	return *this;
		//}

		[[nodiscard]] xCoordTransP44 operator * (xCoordTransP44 const& B) const {
			xCoordTransP44 newChain;
			newChain.m_mat = m_mat * B.m_mat;
			return newChain;
		}

		//[[nodiscard]] xCoordTransP44 operator * (ICoordTrans const& B) const {
		//	xCoordTransP44 newChain;
		//	for (auto const& ct : chain_) {
		//		newChain.chain_.push_back(std::move(ct.NewClone()));
		//	}
		//	newChain.chain_.push_back(std::move(B.NewClone()));
		//	return newChain;
		//}

		//friend [[nodiscard]] xCoordTransP44 operator * (ICoordTrans const& A, xCoordTransP44 const& B) {
		//	xCoordTransP44 newChain;
		//	newChain.chain_.push_back(std::move(A.NewClone()));
		//	for (auto const& ct : B.chain_) {
		//		newChain.chain_.push_back(std::move(ct.NewClone()));
		//	}
		//	return newChain;
		//}

		virtual std::unique_ptr<ICoordTrans> GetInverse() const override {
			bool bOK{};
			auto mat = m_mat.inv(cv::DECOMP_LU, &bOK);
			if (!bOK)
				return {};
			return std::make_unique<this_t>(mat);
		}

		bool GetInv(xCoordTransP44& ctI) const {
			bool bOK{};
			auto mat = m_mat.inv(cv::DECOMP_LU, &bOK);
			if (!bOK)
				return false;
			ctI.m_mat = mat;
			return true;
		}

		void clear() {
			m_mat = m_mat.eye();
		}

		[[nodiscard]] bool SetFrom4Points(std::span<point3_t const> ptsSource, std::span<point3_t const> ptsTarget) {
			if ( (ptsSource.size() < 4) or (ptsTarget.size() < 4) )
				return false;

			auto& pts0 = ptsSource;
			auto& pts1 = ptsTarget;

			double dLenSource = pts0[0].Distance(pts1[1]);
			double dLenTarget = pts1[0].Distance(pts1[1]);
			if ( (dLenSource == 0.0) || (dLenTarget == 0.0) )
				return false;

			mat_t m, a, b;
			a(0, 0) = pts0[0].x; a(0, 1) = pts0[1].x; a(0, 2) = pts0[2].x; a(0, 3) = pts0[3].x;
			a(1, 0) = pts0[0].y; a(1, 1) = pts0[1].y; a(1, 2) = pts0[2].y; a(1, 3) = pts0[3].y;
			a(2, 0) = pts0[0].z; a(2, 1) = pts0[1].z; a(2, 2) = pts0[2].z; a(2, 3) = pts0[3].z;
			a(3, 0) = 1.; a(3, 1) = 1.; a(3, 2) = 1.; a(3, 3) = 1.;

			b(0, 0) = pts1[0].x; b(0, 1) = pts1[1].x; b(0, 2) = pts1[2].x; b(0, 3) = pts1[3].x;
			b(1, 0) = pts1[0].y; b(1, 1) = pts1[1].y; b(1, 2) = pts1[2].y; b(1, 3) = pts1[3].y;
			b(2, 0) = pts1[0].z; b(2, 1) = pts1[1].z; b(2, 2) = pts1[2].z; b(2, 3) = pts1[3].z;
			b(3, 0) = 1.; b(3, 1) = 1.; b(3, 2) = 1.; b(3, 3) = 1.;

			auto d = cv::determinant(a);
			if (d == 0.0) {


				//return false;
			}

			bool bOK{};
			m = b * a.inv(cv::DECOMP_LU, &bOK);
			if (!bOK)
				return false;
			m_mat = m;
			return true;
		}

		//-------------------------------------------------------------------------
		// Operation
		//
		virtual [[nodiscard]] point2_t Trans(point2_t const& pt) const override {
			return Trans(point3_t{pt.x, pt.y, 0.});
		}
		virtual [[nodiscard]] point3_t Trans(point3_t const& pt) const override  {
			auto t0 = std::chrono::steady_clock::now();
			point3_t ptNew;
			ptNew.x = m_mat(0, 0) * pt.x + m_mat(0, 1) * pt.y + m_mat(0, 2) * pt.z + m_mat(0, 3);
			ptNew.y = m_mat(1, 0) * pt.x + m_mat(1, 1) * pt.y + m_mat(1, 2) * pt.z + m_mat(1, 3);
			ptNew.z = m_mat(2, 0) * pt.x + m_mat(2, 1) * pt.y + m_mat(2, 2) * pt.z + m_mat(2, 3);
			double d = m_mat(3, 0) * pt.x + m_mat(3, 1) * pt.y + m_mat(3, 2) * pt.z + m_mat(3, 3);
			if ( (d != 0.0) and (d != 1.0) and std::isfinite(d) ) {
				ptNew.x /= d;
				ptNew.y /= d;
				ptNew.z /= d;
			}
			auto t1 = std::chrono::steady_clock::now();
			auto duration = std::chrono::duration<double, std::micro>(t1-t0);
			return ptNew;
		}
		virtual [[nodiscard]] double Trans(double dLength) const override  {
			return std::sqrt(cv::determinant(m_mat))*dLength;
		}
		virtual [[nodiscard]] std::optional<point2_t> TransInverse(point2_t const& pt) const override {
			return TransInverse(point3_t{pt.x, pt.y, 0.0});
		}
		virtual [[nodiscard]] std::optional<point3_t> TransInverse(point3_t const& pt) const override {
			this_t ctI;
			if (!GetInv(ctI))
				return {};
			return ctI.Trans(pt);
		}
		virtual [[nodiscard]] std::optional<double> TransInverse(double dLength) const override {
			double d = cv::determinant(m_mat);
			if (d == 0)
				return {};
			return dLength / std::sqrt(d);
		}

		virtual [[nodiscard]] bool IsRightHanded() const override {
			bool bRightHanded{cv::determinant(m_mat) > 0};
			return bRightHanded;
		}

		static inline [[nodiscard]] mat_t GetRotatingMatrixXY(rad_t angle) {
			double c{cos(angle)}, s{sin(angle)};
			//return mat_t{c, -s, 0., 0., /**/ s, c, 0., 0., /**/ 0., 0., 1., 0., /**/ 0., 0., 0., 1. };
		}
		static inline [[nodiscard]] mat_t GetRotatingMatrixYZ(rad_t angle) {
			double c{cos(angle)}, s{sin(angle)};
			//return mat_t{1., 0., 0., 0., /**/ 0., c, -s, 0., /**/ 0., s, c, 0., /**/ 0., 0., 0., 1. };
		}
		static inline [[nodiscard]] mat_t GetRotatingMatrixZX(rad_t angle) {
			double c{cos(angle)}, s{sin(angle)};
			//return mat_t{c, 0., s, 0., /**/ 0., 1., 0., 0., /**/ -s, 0., c, 0., /**/ 0., 0., 0., 1., };
		}

	};

#pragma pack(pop)
}	// namespace gtl

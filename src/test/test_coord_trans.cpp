#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/coord/coord_trans.h"
#include "gtl/coord/coord_trans_perspective.h"


using namespace std::literals;
using namespace gtl::literals;

TEST(gtl_coord_trans, ct2) {
	using namespace gtl;
	{
		xCoordTrans2d ct;

		auto p1 = ct(POINT{1, 2});
		auto p2 = POINT{1, 2};
		EXPECT_EQ(p1.x, p2.x);
		EXPECT_EQ(p1.y, p2.y);
	}

	xPoint2d pt{1, 2};
	xCoordTrans2d ct{1, {0, 1, 1, 0}, {}, {}};

	EXPECT_EQ(ct(pt), xPoint2d(2, 1));

	std::vector<xPoint2d> pts0{ {0, 0}, {1, 0}, {0, 1}, };
	std::vector<xPoint2d> pts1{ {0, 0}, {0, 1}, {1, 0}, };
	EXPECT_TRUE(ct.SetFrom2Points(pts0, pts1, true, 0.0, false));
	EXPECT_EQ(ct(pts0[0]), pts1[0]);
	EXPECT_EQ(ct(pts0[1]), pts1[1]);
	EXPECT_EQ(ct(pts0[2]), pts1[2]);

	EXPECT_TRUE(ct.SetFrom3Points(pts0, pts1));
	EXPECT_EQ(ct.m_scale, 1.0);
	EXPECT_EQ(ct.m_mat, (cv::Matx22d{0, 1, 1, 0}));
	EXPECT_EQ(ct.m_origin, xPoint2d(0, 0));
	EXPECT_EQ(ct.m_offset, xPoint2d(0, 0));
	EXPECT_EQ(ct(pts0[0]), pts1[0]);
	EXPECT_EQ(ct(pts0[1]), pts1[1]);
	EXPECT_EQ(ct(pts0[2]), pts1[2]);
	EXPECT_EQ(ct.TransI(pts1[0]), pts0[0]);
	EXPECT_EQ(ct.TransI(pts1[1]), pts0[1]);
	EXPECT_EQ(ct.TransI(pts1[2]), pts0[2]);


}

TEST(gtl_coord_trans, ct3) {
	using namespace gtl;
	xPoint3d pt{1, 2, 3};
	xCoordTrans3d ct{1, {0, 0, 1, 0, 1, 0, 1, 0, 0}, {}, {}};

	EXPECT_EQ(ct(pt), xPoint3d(3, 2, 1));

	std::vector<xPoint3d> pts0{ {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
	std::vector<xPoint3d> pts1{ {0, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 0} };
	EXPECT_TRUE(ct.SetFrom4Points(pts0, pts1));
	EXPECT_EQ(ct(pts0[0]), pts1[0]);
	EXPECT_EQ(ct(pts0[1]), pts1[1]);
	EXPECT_EQ(ct(pts0[2]), pts1[2]);
	EXPECT_EQ(ct(pts0[3]), pts1[3]);
	EXPECT_EQ(ct.TransI(pts1[0]), pts0[0]);
	EXPECT_EQ(ct.TransI(pts1[1]), pts0[1]);
	EXPECT_EQ(ct.TransI(pts1[2]), pts0[2]);
	EXPECT_EQ(ct.TransI(pts1[3]), pts0[3]);

}

TEST(gtl_coord_trans_perspective, ctP33) {
	using namespace gtl;
	xPoint2d pt{2, 3};
	xCoordTransP33 ct(cv::Matx33d{0., 1., 0., /**/ 1., 0., 0., /**/ 0., 0., 1.});

	auto pt2 = ct(pt);
	EXPECT_EQ(ct(pt), xPoint2d(3, 2));

	std::vector<xPoint2d> pts0{ {0., 0.}, {1., 0.}, {0., 1.}, {1., 1.} };
	std::vector<xPoint2d> pts1{ {0., 0.}, {1., 0.}, {0., 1.}, {1., 1.} };
	EXPECT_TRUE(ct.SetFrom4Points(pts0, pts1));

	auto v0 = ct(pts0[0]);
	auto v1 = ct(pts0[1]);
	auto v2 = ct(pts0[2]);
	auto v3 = ct(pts0[3]);
	auto v4 = ct.TransI(pts1[0]);
	auto v5 = ct.TransI(pts1[1]);
	auto v6 = ct.TransI(pts1[2]);
	auto v7 = ct.TransI(pts1[3]);


	EXPECT_EQ(ct(pts0[0]), pts1[0]);
	EXPECT_EQ(ct(pts0[1]), pts1[1]);
	EXPECT_EQ(ct(pts0[2]), pts1[2]);
	EXPECT_EQ(ct(pts0[3]), pts1[3]);
	EXPECT_EQ(ct.TransI(pts1[0]), pts0[0]);
	EXPECT_EQ(ct.TransI(pts1[1]), pts0[1]);
	EXPECT_EQ(ct.TransI(pts1[2]), pts0[2]);
	EXPECT_EQ(ct.TransI(pts1[3]), pts0[3]);

}

TEST(gtl_coord_trans_perspective, ctP33_2) {
	using namespace gtl;
	xPoint2d pt{2, 3};
	xCoordTransP33 ct;

	std::vector<xPoint2d> pts0{ {0., 0.},  {1., 0.}, {0., 1.}, {1., 1.} };
	std::vector<xPoint2d> pts1{ {0., 0.1}, {0.9, 0.}, {0., 1.}, {1., 1.} };
	EXPECT_TRUE(ct.SetFrom4Points(pts0, pts1));

	auto v0 = ct(pts0[0]);
	auto v1 = ct(pts0[1]);
	auto v2 = ct(pts0[2]);
	auto v3 = ct(pts0[3]);
	auto v4 = ct.TransI(pts1[0]);
	auto v5 = ct.TransI(pts1[1]);
	auto v6 = ct.TransI(pts1[2]);
	auto v7 = ct.TransI(pts1[3]);


	EXPECT_TRUE(ct(pts0[0]).Distance(pts1[0]) < 1.e-6);
	EXPECT_TRUE(ct(pts0[1]).Distance(pts1[1]) < 1.e-6);
	EXPECT_TRUE(ct(pts0[2]).Distance(pts1[2]) < 1.e-6);
	EXPECT_TRUE(ct(pts0[3]).Distance(pts1[3]) < 1.e-6);
	EXPECT_TRUE(ct.TransI(pts1[0]).Distance(pts0[0]) < 1.e-6);
	EXPECT_TRUE(ct.TransI(pts1[1]).Distance(pts0[1]) < 1.e-6);
	EXPECT_TRUE(ct.TransI(pts1[2]).Distance(pts0[2]) < 1.e-6);
	EXPECT_TRUE(ct.TransI(pts1[3]).Distance(pts0[3]) < 1.e-6);

}

TEST(gtl_coord_trans_perspective, ctP33_3) {
	using namespace gtl;
	xPoint2d pt{2, 3};
	xCoordTransP33 ct;

	std::vector<xPoint2d> pts0{ {0., 0.},  {100., 0.}, {0., 100.}, {100., 100.} };
	std::vector<xPoint2d> pts1{ {0., 0.1}, {100.9, 0.}, {0., 100.}, {100., 100.} };
	EXPECT_TRUE(ct.SetFrom4Points(pts0, pts1));

	auto v0 = ct(pts0[0]);			auto d0 = v0.Distance(pts1[0]);
	auto v1 = ct(pts0[1]);			auto d1 = v1.Distance(pts1[1]);
	auto v2 = ct(pts0[2]);			auto d2 = v2.Distance(pts1[2]);
	auto v3 = ct(pts0[3]);			auto d3 = v3.Distance(pts1[3]);
	auto v4 = ct.TransI(pts1[0]);	auto d4 = v4.Distance(pts0[0]);
	auto v5 = ct.TransI(pts1[1]);	auto d5 = v5.Distance(pts0[1]);
	auto v6 = ct.TransI(pts1[2]);	auto d6 = v6.Distance(pts0[2]);
	auto v7 = ct.TransI(pts1[3]);	auto d7 = v7.Distance(pts0[3]);


	EXPECT_TRUE(ct(pts0[0]).Distance(pts1[0]) < 1.e-5);
	EXPECT_TRUE(ct(pts0[1]).Distance(pts1[1]) < 1.e-5);
	EXPECT_TRUE(ct(pts0[2]).Distance(pts1[2]) < 1.e-5);
	EXPECT_TRUE(ct(pts0[3]).Distance(pts1[3]) < 1.e-5);
	EXPECT_TRUE(ct.TransI(pts1[0]).Distance(pts0[0]) < 1.e-5);
	EXPECT_TRUE(ct.TransI(pts1[1]).Distance(pts0[1]) < 1.e-5);
	EXPECT_TRUE(ct.TransI(pts1[2]).Distance(pts0[2]) < 1.e-5);
	EXPECT_TRUE(ct.TransI(pts1[3]).Distance(pts0[3]) < 1.e-5);

}

TEST(gtl_coord_trans_perspective, ctP44) {
	using namespace gtl;
	xPoint3d pt{1, 2, 3};
	xCoordTransP44 ct(xCoordTransP44::mat_t{0., 0., 1., 0., /**/ 0., 1., 0., 0., /**/ 1., 0., 0., 0., /**/ 0., 0., 0., 1.});

	EXPECT_EQ(ct(pt), xPoint3d(3, 2, 1));

	std::vector<xPoint3d> pts0{ {0., 0., 0.}, {1., 0., 0.}, {0., 1., 0.}, {0., 0., 1.} };
	std::vector<xPoint3d> pts1{ {0., 0., 0.}, {1., 0., 0.}, {0., 1., 0.}, {0., 0., 1.} };
	EXPECT_TRUE(ct.SetFrom4Points(pts0, pts1));

	auto v0 = ct(pts0[0]);
	auto v1 = ct(pts0[1]);
	auto v2 = ct(pts0[2]);
	auto v3 = ct(pts0[3]);
	auto v4 = ct.TransI(pts1[0]);
	auto v5 = ct.TransI(pts1[1]);
	auto v6 = ct.TransI(pts1[2]);
	auto v7 = ct.TransI(pts1[3]);


	EXPECT_EQ(ct(pts0[0]), pts1[0]);
	EXPECT_EQ(ct(pts0[1]), pts1[1]);
	EXPECT_EQ(ct(pts0[2]), pts1[2]);
	EXPECT_EQ(ct(pts0[3]), pts1[3]);
	EXPECT_EQ(ct.TransI(pts1[0]), pts0[0]);
	EXPECT_EQ(ct.TransI(pts1[1]), pts0[1]);
	EXPECT_EQ(ct.TransI(pts1[2]), pts0[2]);
	EXPECT_EQ(ct.TransI(pts1[3]), pts0[3]);

}

TEST(gtl_coord_trans_perspective, ctP44_2) {
	using namespace gtl;
	xPoint3d pt{1, 2, 3};
	xCoordTransP44 ct(xCoordTransP44::mat_t{0., 0., 1., 0., /**/ 0., 1., 0., 0., /**/ 1., 0., 0., 0., /**/ 0., 0., 0., 1.});

	EXPECT_EQ(ct(pt), xPoint3d(3, 2, 1));

	std::vector<xPoint3d> pts0{ {0., 0., 0.}, {1., 0., 0.}, {0., 1., 0.}, {0., 0., 1.} };
	std::vector<xPoint3d> pts1{ {0., 0., 1.}, {0., 2., 1.}, {3., 0., 1.}, {0., 0., 2.} };
	EXPECT_TRUE(ct.SetFrom4Points(pts0, pts1));

	auto v0 = ct(pts0[0]);
	auto v1 = ct(pts0[1]);
	auto v2 = ct(pts0[2]);
	auto v3 = ct(pts0[3]);
	auto v4 = ct.TransI(pts1[0]);
	auto v5 = ct.TransI(pts1[1]);
	auto v6 = ct.TransI(pts1[2]);
	auto v7 = ct.TransI(pts1[3]);


	EXPECT_EQ(ct(pts0[0]), pts1[0]);
	EXPECT_EQ(ct(pts0[1]), pts1[1]);
	EXPECT_EQ(ct(pts0[2]), pts1[2]);
	EXPECT_EQ(ct(pts0[3]), pts1[3]);
	EXPECT_EQ(ct.TransI(pts1[0]), pts0[0]);
	EXPECT_EQ(ct.TransI(pts1[1]), pts0[1]);
	EXPECT_EQ(ct.TransI(pts1[2]), pts0[2]);
	EXPECT_EQ(ct.TransI(pts1[3]), pts0[3]);

}

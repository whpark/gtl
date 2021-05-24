#include "pch.h"

#include <windows.h>

#include "gtl/gtl.h"
#include "gtl/coord/coord_trans.h"


using namespace std::literals;
using namespace gtl::literals;

TEST(gtl_coord_trans, ct2) {
	using namespace gtl;
	{
		CCoordTrans2d ct;

		auto p1 = ct(POINT{1, 2});
		auto p2 = POINT{1, 2};
		EXPECT_EQ(p1.x, p2.x);
		EXPECT_EQ(p1.y, p2.y);
	}

	CPoint2d pt{1, 2};
	CCoordTrans2d ct{1, {0, 1, 1, 0}, {}, {}};

	EXPECT_EQ(ct(pt), CPoint2d(2, 1));

	std::vector<CPoint2d> pts0{ {0, 0}, {1, 0}, {0, 1}, };
	std::vector<CPoint2d> pts1{ {0, 0}, {0, 1}, {1, 0}, };
	EXPECT_TRUE(ct.SetFrom2Points(pts0, pts1, true, 0.0, false));
	EXPECT_EQ(ct(pts0[0]), pts1[0]);
	EXPECT_EQ(ct(pts0[1]), pts1[1]);
	EXPECT_EQ(ct(pts0[2]), pts1[2]);

	EXPECT_TRUE(ct.SetFrom3Points(pts0, pts1));
	EXPECT_EQ(ct.scale_, 1.0);
	EXPECT_EQ(ct.mat_, (cv::Matx22d{0, 1, 1, 0}));
	EXPECT_EQ(ct.origin_, CPoint2d(0, 0));
	EXPECT_EQ(ct.offset_, CPoint2d(0, 0));
	EXPECT_EQ(ct(pts0[0]), pts1[0]);
	EXPECT_EQ(ct(pts0[1]), pts1[1]);
	EXPECT_EQ(ct(pts0[2]), pts1[2]);
	EXPECT_EQ(ct.TransI(pts1[0]), pts0[0]);
	EXPECT_EQ(ct.TransI(pts1[1]), pts0[1]);
	EXPECT_EQ(ct.TransI(pts1[2]), pts0[2]);


}

TEST(gtl_coord_trans, ct3) {
	using namespace gtl;
	CPoint3d pt{1, 2, 3};
	CCoordTrans3d ct{1, {0, 0, 1, 0, 1, 0, 1, 0, 0}, {}, {}};

	EXPECT_EQ(ct(pt), CPoint3d(3, 2, 1));

	std::vector<CPoint3d> pts0{ {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
	std::vector<CPoint3d> pts1{ {0, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 0} };
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

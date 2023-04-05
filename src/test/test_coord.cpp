#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/coord.h"


using namespace std::literals;
using namespace gtl::literals;

TEST(gtl_coord, point) {
	using namespace gtl;

	xPoint3d pt;
	EXPECT_EQ(sizeof(pt), sizeof(decltype(pt)::coord_t));
	static_assert(sizeof(pt) == sizeof(decltype(pt)::coord_t));
	int value{};
	for (auto& v : pt.arr()) {
		v = ++value;
	}
	EXPECT_EQ(pt.x, 1);
	EXPECT_EQ(pt.y, 2);
	EXPECT_EQ(pt.z, 3);

	for (int i{}; i < pt.size(); i++) {
		pt.data()[i] = ++value;
	}
	EXPECT_EQ(pt.x, 4);
	EXPECT_EQ(pt.y, 5);
	EXPECT_EQ(pt.z, 6);

	for (int i{}; i < pt.size(); i++) {
		pt.member(i) = ++value;
	}
	EXPECT_EQ(pt.x, 7);
	EXPECT_EQ(pt.y, 8);
	EXPECT_EQ(pt.z, 9);

	static_assert(xPoint3i{1.2, 2.5, -0.6} == xPoint3i{1, 3, -1});
	static_assert(xPoint3i{1.2, -2.5, -0.6} == xPoint3i{1, -3, -1});
	static_assert(xPoint3d::All(3.1415) == xPoint3d{3.1415, 3.1415, 3.1415});
	static_assert(xPoint3d::All(3.1415) != xPoint3d{3.141562, 3.1415, 3.1415});

	cv::Point3d ptCV = xPoint3d{1.2, 2.5, -0.6};
	EXPECT_EQ(ptCV, cv::Point3d(1.2, 2.5, -0.6));

	cv::Point3i ptiCV = xPoint3d{1.2, 2.5, -0.6};
	EXPECT_EQ(ptiCV, cv::Point3i(1, 3, -1));

	{
		POINT ptWnd = xPoint3d{3.7, -1.6};
		EXPECT_EQ(ptWnd.x, 4);
		EXPECT_EQ(ptWnd.y, -2);
	}

	EXPECT_EQ(xPoint3d{}.CountNonZero(), 0);
	EXPECT_EQ((xPoint3d{1, 0, 0}.CountNonZero()), 1);
	EXPECT_EQ((xPoint3d{1, 0, 2}.CountNonZero()), 2);

	EXPECT_EQ(xPoint3d{}.CountIf([](auto v){ return v == 0; }), 3);
	EXPECT_EQ((xPoint3d{1, 0, 0}.CountNonZero()), 1);
	EXPECT_EQ((xPoint3d{1, 0, 2}.CountNonZero()), 2);

	
	EXPECT_EQ((xPoint3d{1, 2, 3} += {1, 2, 3}), (xPoint3d{2, 4, 6}));
	EXPECT_EQ((xPoint3d{1, 2, 3} -= {3, 2, 1}), (xPoint3d{-2, 0, 2}));
	EXPECT_EQ((xPoint3d{1, 2, 3} + xPoint3d{1, 2, 3}), (xPoint3d{2, 4, 6}));
	EXPECT_EQ((xPoint3d{1, 2, 3} - xPoint3d{3, 2, 1}), (xPoint3d{-2, 0, 2}));
	EXPECT_EQ(-xPoint3d(1, 2, 3), xPoint3d(-1, -2, -3));

	EXPECT_EQ((xPoint3d(1, 2, 3) += cv::Point3d(1, 2, 3)), xPoint3d(2, 4, 6));
	EXPECT_EQ((xPoint3d(1, 2, 3) + cv::Point3d(2, 4.1, 7)), xPoint3d(3, 6.1, 10));
	EXPECT_EQ((xPoint3d(1, 2, 3) + xSize3d(2, 4.1, 7)), xPoint3d(3, 6.1, 10));
	EXPECT_EQ((xPoint3d(1, 2, 3) += POINT{1, 2}), xPoint3d(2, 4, 3));
	EXPECT_EQ((xPoint3d(1, 2, 3) + POINT{1, 2}), xPoint3d(2, 4, 3));
	EXPECT_EQ((xPoint3d(1, 2, 3) += SIZE{1, 2}), xPoint3d(2, 4, 3));
	EXPECT_EQ((xPoint3d(1, 2, 3) + cv::Size2d(2, 4.1)), xPoint3d(3, 6.1, 3));

	EXPECT_EQ((xPoint3d(1, 2, 3) -= cv::Point3d(3, 2, 1)), xPoint3d(-2, 0, 2));
	EXPECT_EQ((xPoint3d(1, 2.1, 3) - cv::Point3d(2, 4, 7)), xPoint3d(-1, -1.9, -4));
	EXPECT_EQ((xPoint3d(1, 2.1, 3) - xSize3d(2, 4, 7)), xPoint3d(-1, -1.9, -4));
	EXPECT_EQ((xPoint3d(1, 2, 3) -= POINT{1, 2}), xPoint3d(0, 0, 3));
	EXPECT_EQ((xPoint3d(1, 2, 3) - POINT{1, 2}), xPoint3d(0, 0, 3));
	EXPECT_EQ((xPoint3d(1, 2, 3) -= SIZE{1, 2}), xPoint3d(0, 0, 3));
	EXPECT_EQ((xPoint3d(1, 2.1, 3) - cv::Size2d(2, 4)), xPoint3d(-1, -1.9, 3));


	EXPECT_EQ((xPoint3d{1, 2, 3} *= 3), xPoint3d(3, 6, 9));
	EXPECT_EQ((xPoint3d{3, 6, 9} /= 3), xPoint3d(1, 2, 3));
	EXPECT_EQ((xPoint3d{1, 2, 3} += 10), xPoint3d(11, 12, 13));
	EXPECT_EQ((xPoint3d{1, 2, 3} -= 10), xPoint3d(-9, -8, -7));

	EXPECT_EQ((xPoint3d{1, 2, 3} * 3), xPoint3d(3, 6, 9));
	EXPECT_EQ((xPoint3d{3, 6, 9} / 3), xPoint3d(1, 2, 3));
	EXPECT_EQ((xPoint3d{1, 2, 3} + 10), xPoint3d(11, 12, 13));
	EXPECT_EQ((xPoint3d{1, 2, 3} - 10), xPoint3d(-9, -8, -7));

	EXPECT_EQ((3 * xPoint3d{1, 2, 3}), xPoint3d(3, 6, 9));
	EXPECT_EQ((6 / xPoint3d{1, 2, 3}), xPoint3d(6, 3, 2));
	EXPECT_EQ((3 + xPoint3d{1, 2, 3}), xPoint3d(4, 5, 6));
	EXPECT_EQ((3 - xPoint3d{1, 2, 3}), xPoint3d(2, 1, 0));

	EXPECT_EQ((xPoint3d{1, 0, 0} * xPoint3d{0, 1, 0}), xPoint3d(0, 0, 1));
	EXPECT_EQ((xPoint3d{0, 1, 0} * xPoint3d{0, 0, 1}), xPoint3d(1, 0, 0));
	EXPECT_EQ((xPoint3d{0, 0, 1} * xPoint3d{1, 0, 0}), xPoint3d(0, 1, 0));
	EXPECT_EQ((xPoint3d{0, 1, 0} * xPoint3d{1, 0, 0}), xPoint3d(0, 0, -1));
	EXPECT_EQ((xPoint3d{0, 0, 1} * xPoint3d{0, 1, 0}), xPoint3d(-1, 0, 0));
	EXPECT_EQ((xPoint3d{1, 0, 0} * xPoint3d{0, 0, 1}), xPoint3d(0, -1, 0));
	EXPECT_EQ((xPoint3d(3, -3, 1) * xPoint3d(4, 9, 2)), xPoint3d(-15, -2, 39));

	auto matEye = cv::Matx33d::eye();
	EXPECT_EQ(matEye * xPoint3d(1, 2, 3), xPoint3d(1, 2, 3));
	EXPECT_EQ(matEye * pt, pt);
	auto matOnes = cv::Matx33d::ones();
	EXPECT_EQ(matOnes * xPoint3d(1, 2, 4), xPoint3d(7, 7, 7));

	pt.Set(3, 44, 2);
	EXPECT_EQ(pt, xPoint3d(3, 44, 2));
	pt.SetAll(200);
	EXPECT_EQ(pt, xPoint3d(200, 200, 200));

	pt.SetAsNAN();
	EXPECT_FALSE(pt.IsAllValid());

	xPoint3d ptMin(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()),
		ptMax(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest());
	std::vector<xPoint3d> pts{ {0, 0, 0}, {-10, 3, 2}, { 10, 4, 3}, {2, -10, 2}, {3, 9, 1}, { 0, 10, 1}, { 0, 0, 10}, {3, 9, -10}, };
	for (auto const& pt : pts) {
		pt.CheckMinMax(ptMin, ptMax);
	}
	EXPECT_EQ(ptMin, xPoint3d(-10, -10, -10));
	EXPECT_EQ(ptMax, xPoint3d(10, 10, 10));

	nlohmann::json j;
	j = xPoint3d(100, 200, 300);

	xPoint3d ptJ;
	ptJ = j;
	EXPECT_EQ(ptJ, xPoint3d(100, 200, 300));

	EXPECT_EQ(xPoint3d(1, 0, 0).GetLength(), 1.);
	EXPECT_EQ(xPoint3d(1, 1, 0).GetLength(), sqrt(2.));
	EXPECT_EQ(xPoint3d(1, 1, 1).GetLength(), sqrt(3.));
	EXPECT_EQ(xPoint3d(1, 2, 3).GetLength(), sqrt(14.));
	EXPECT_EQ(xPoint3d(3, 4, 0).GetLength(), 5);
	EXPECT_EQ(xPoint3d(5, 12, 0).GetLength(), 13);
	EXPECT_EQ(xPoint3d(0, 5, 12).GetLength(), 13);
	EXPECT_EQ(xPoint3d(5, 0, 12).GetLength(), 13);

	EXPECT_EQ(xPoint3d(5, 13, 3).Distance({0, 1, 3}), 13);

	EXPECT_EQ(xPoint3d(1, 1, 0).GetAngleXY(),				(rad_t)45_deg);
	EXPECT_EQ(xPoint3d(1, std::sqrt(3.), 0).GetAngleXY(),	(rad_t)60_deg);
	EXPECT_EQ(xPoint3d(0, 1, 1).GetAngleYZ(),				(rad_t)45_deg);
	EXPECT_EQ(xPoint3d(0, 1, std::sqrt(3.)).GetAngleYZ(),	(rad_t)60_deg);
	EXPECT_EQ(xPoint3d(1, 0, 1).GetAngleZX(),				(rad_t)45_deg);
	EXPECT_EQ(xPoint3d(std::sqrt(3.), 0, 1).GetAngleZX(),	(rad_t)60_deg);

	EXPECT_EQ(xPoint3d(1, 1, 1).GetNormalizedVector(), xPoint3d::All(1/std::sqrt(3.)));
	EXPECT_EQ(xPoint3d(1, 2, 3).GetNormalizedVector(), xPoint3d(1/std::sqrt(14.), 2/std::sqrt(14.), 3/std::sqrt(14.)));

	EXPECT_EQ(xPoint3d(1, 2, 3).Dot(xPoint3d(3, 4, 5)), 26);

}

TEST(gtl_coord, size) {
	using namespace gtl;

	xSize3d pt;
	EXPECT_EQ(sizeof(pt), sizeof(decltype(pt)::coord_t));
	static_assert(sizeof(pt) == sizeof(decltype(pt)::coord_t));
	int value{};
	for (auto& v : pt.arr()) {
		v = ++value;
	}
	EXPECT_EQ(pt.cx, 1);
	EXPECT_EQ(pt.cy, 2);
	EXPECT_EQ(pt.cz, 3);

	for (int i{}; i < pt.size(); i++) {
		pt.data()[i] = ++value;
	}
	EXPECT_EQ(pt.cx, 4);
	EXPECT_EQ(pt.cy, 5);
	EXPECT_EQ(pt.cz, 6);

	for (int i{}; i < pt.size(); i++) {
		pt.member(i) = ++value;
	}
	EXPECT_EQ(pt.cx, 7);
	EXPECT_EQ(pt.cy, 8);
	EXPECT_EQ(pt.cz, 9);

	static_assert(xPoint3i{1.2, 2.5, -0.6} == xPoint3i{1, 3, -1});
	static_assert(xPoint3i{1.2, -2.5, -0.6} == xPoint3i{1, -3, -1});
	static_assert(xSize3d::All(3.1415) == xSize3d{3.1415, 3.1415, 3.1415});
	static_assert(xSize3d::All(3.1415) != xSize3d{3.141562, 3.1415, 3.1415});

	cv::Point3d ptCV = xSize3d{1.2, 2.5, -0.6};
	EXPECT_EQ(ptCV, cv::Point3d(1.2, 2.5, -0.6));

	cv::Size2i ptiCV = xSize3d{1.2, 2.5, -0.6};
	EXPECT_EQ(ptiCV, cv::Size2i(1, 3));

	{
		SIZE sizeWnd = xSize3d{3.7, -1.6, 32};
		EXPECT_EQ(sizeWnd.cx, 4);
		EXPECT_EQ(sizeWnd.cy, -2);
	}

	EXPECT_EQ(xSize3d{}.CountNonZero(), 0);
	EXPECT_EQ((xSize3d{1, 0, 0}.CountNonZero()), 1);
	EXPECT_EQ((xSize3d{1, 0, 2}.CountNonZero()), 2);

	EXPECT_EQ(xSize3d{}.CountIf([](auto v){ return v == 0; }), 3);
	EXPECT_EQ((xSize3d{1, 0, 0}.CountNonZero()), 1);
	EXPECT_EQ((xSize3d{1, 0, 2}.CountNonZero()), 2);


	EXPECT_EQ((xSize3d{1, 2, 3} += {1, 2, 3}), (xSize3d{2, 4, 6}));
	EXPECT_EQ((xSize3d{1, 2, 3} -= {3, 2, 1}), (xSize3d{-2, 0, 2}));
	EXPECT_EQ((xSize3d{1, 2, 3} + xSize3d{1, 2, 3}), (xSize3d{2, 4, 6}));
	EXPECT_EQ((xSize3d{1, 2, 3} - xSize3d{3, 2, 1}), (xSize3d{-2, 0, 2}));
	EXPECT_EQ(-xSize3d(1, 2, 3), xSize3d(-1, -2, -3));

	EXPECT_EQ((xSize3d(1, 2, 3) += cv::Point3d(1, 2, 3)), xSize3d(2, 4, 6));
	EXPECT_EQ((xSize3d(1, 2, 3) + cv::Point3d(2, 4.1, 7)), xSize3d(3, 6.1, 10));
	EXPECT_EQ((xSize3d(1, 2, 3) + xSize3d(2, 4.1, 7)), xSize3d(3, 6.1, 10));
	EXPECT_EQ((xSize3d(1, 2, 3) += POINT{1, 2}), xSize3d(2, 4, 3));
	EXPECT_EQ((xSize3d(1, 2, 3) + POINT{1, 2}), xSize3d(2, 4, 3));
	EXPECT_EQ((xSize3d(1, 2, 3) += SIZE{1, 2}), xSize3d(2, 4, 3));
	EXPECT_EQ((xSize3d(1, 2, 3) + cv::Size2d(2, 4.1)), xSize3d(3, 6.1, 3));

	EXPECT_EQ((xSize3d(1, 2, 3) -= cv::Point3d(3, 2, 1)), xSize3d(-2, 0, 2));
	EXPECT_EQ((xSize3d(1, 2.1, 3) - cv::Point3d(2, 4, 7)), xSize3d(-1, -1.9, -4));
	EXPECT_EQ((xSize3d(1, 2.1, 3) - xSize3d(2, 4, 7)), xSize3d(-1, -1.9, -4));
	EXPECT_EQ((xSize3d(1, 2, 3) -= POINT{1, 2}), xSize3d(0, 0, 3));
	EXPECT_EQ((xSize3d(1, 2, 3) - POINT{1, 2}), xSize3d(0, 0, 3));
	EXPECT_EQ((xSize3d(1, 2, 3) -= SIZE{1, 2}), xSize3d(0, 0, 3));
	EXPECT_EQ((xSize3d(1, 2.1, 3) - cv::Size2d(2, 4)), xSize3d(-1, -1.9, 3));


	EXPECT_EQ((xSize3d{1, 2, 3} *= 3), xSize3d(3, 6, 9));
	EXPECT_EQ((xSize3d{3, 6, 9} /= 3), xSize3d(1, 2, 3));
	EXPECT_EQ((xSize3d{1, 2, 3} += 10), xSize3d(11, 12, 13));
	EXPECT_EQ((xSize3d{1, 2, 3} -= 10), xSize3d(-9, -8, -7));

	EXPECT_EQ((xSize3d{1, 2, 3} * 3), xSize3d(3, 6, 9));
	EXPECT_EQ((xSize3d{3, 6, 9} / 3), xSize3d(1, 2, 3));
	EXPECT_EQ((xSize3d{1, 2, 3} + 10), xSize3d(11, 12, 13));
	EXPECT_EQ((xSize3d{1, 2, 3} - 10), xSize3d(-9, -8, -7));

	EXPECT_EQ((3 * xSize3d{1, 2, 3}), xSize3d(3, 6, 9));
	EXPECT_EQ((6 / xSize3d{1, 2, 3}), xSize3d(6, 3, 2));
	EXPECT_EQ((3 + xSize3d{1, 2, 3}), xSize3d(4, 5, 6));
	EXPECT_EQ((3 - xSize3d{1, 2, 3}), xSize3d(2, 1, 0));

	pt.Set(3, 44, 2);
	EXPECT_EQ(pt, xSize3d(3, 44, 2));
	pt.SetAll(200);
	EXPECT_EQ(pt, xSize3d(200, 200, 200));

	pt.SetAsNAN();
	EXPECT_FALSE(pt.IsAllValid());

	xSize3d ptMin(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()),
		ptMax(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest());
	std::vector<xSize3d> pts{ {0, 0, 0}, {-10, 3, 2}, { 10, 4, 3}, {2, -10, 2}, {3, 9, 1}, { 0, 10, 1}, { 0, 0, 10}, {3, 9, -10}, };
	for (auto const& pt : pts) {
		pt.CheckMinMax(ptMin, ptMax);
	}
	EXPECT_EQ(ptMin, xSize3d(-10, -10, -10));
	EXPECT_EQ(ptMax, xSize3d(10, 10, 10));

	nlohmann::json j;
	j = xSize3d(100, 200, 300);

	xSize3d ptJ;
	ptJ = j;
	EXPECT_EQ(ptJ, xSize3d(100, 200, 300));

}

TEST(gtl_coord, rect) {
	using namespace gtl;

	xPoint3d pt{1., 2., 3};
	xSize3d size{100., 200., 300.};
	xRect3d rect{pt, size};
	EXPECT_EQ(rect.Width(), 100);
	EXPECT_EQ(rect.Height(), 200);
	EXPECT_EQ(rect.Depth(), 300);

	{
		xPoint3d pt0{ 1, 2, 3 };
		xPoint3d pt1{ 101, 202, 303 };
		xRect3d rect2{ pt0, pt1 };
		EXPECT_EQ(rect2.Width(), 100);
		EXPECT_EQ(rect2.Height(), 200);
		EXPECT_EQ(rect2.Depth(), 300);
	}

	{
		xPoint2d pt0{ 1, 2 };
		xPoint2d pt1{ 101, 202 };
		xRect2d rect2{ pt0, pt1 };
		EXPECT_EQ(rect2.Width(), 100);
		EXPECT_EQ(rect2.Height(), 200);
	}

}

TEST(gtl_coord, srect) {
	using namespace gtl;

	{
		xPoint3d pt{ 1., 2., 3 };
		xSize3d size{ 100., 200., 300. };
		xSRect3d rect{ pt, size };
		EXPECT_EQ(rect.Width(), 100);
		EXPECT_EQ(rect.Height(), 200);
		EXPECT_EQ(rect.Depth(), 300);
	}

	{
		xSRect3d rect(xRect2d(10, 20, 110, 220));
		EXPECT_EQ(rect.x, 10.);
		EXPECT_EQ(rect.y, 20.);
		EXPECT_EQ(rect.width, 100.);
		EXPECT_EQ(rect.height, 200.);
	}

	{
		xRect3d rect(xSRect2d(10, 20, 100, 200));
		EXPECT_EQ(rect.left, 10.);
		EXPECT_EQ(rect.top, 20.);
		EXPECT_EQ(rect.right, 110.);
		EXPECT_EQ(rect.bottom, 220.);
	}

	{
		xPoint3d pt0{ 1, 2, 3 };
		xPoint3d pt1{ 101, 202, 303 };
		xSRect3d rect2{ pt0, pt1 };
		EXPECT_EQ(rect2.Width(), 100);
		EXPECT_EQ(rect2.Height(), 200);
		EXPECT_EQ(rect2.Depth(), 300);
	}

	{
		xPoint2d pt0{ 1, 2 };
		xPoint2d pt1{ 101, 202 };
		xSRect2d rect2{ pt0, pt1 };
		EXPECT_EQ(rect2.Width(), 100);
		EXPECT_EQ(rect2.Height(), 200);
	}

}

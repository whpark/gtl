#include "pch.h"

#include "gtl/gtl.h"

#include "gtl/shape/shape.h"
#define FMT_HEADER_ONLY
#include "fmt/chrono.h"

using namespace std::literals;
using namespace gtl::literals;

#ifdef _DEBUG
#define DEBUG_PRINT(...) fmt::print(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) 
#endif

TEST(gtl_shape, basic) {
	std::filesystem::path paths[] = {
		LR"xx(shape_test/bridge.dxf.json)xx",
		LR"xx(shape_test/cube.dxf.json)xx",
		LR"xx(shape_test/diamond.dxf.json)xx",
	};

	auto rLine1 = std::make_unique<gtl::shape::xLine>();
	auto rLine2 = std::make_unique<gtl::shape::xLine>();
	rLine1->m_pt0 = {1, 1};
	rLine1->m_pt1 = {2, 2};
	rLine2->m_pt0 = {1, 1};
	rLine2->m_pt1 = {2, 2};
	EXPECT_EQ(*rLine1, *rLine2);

	rLine2->m_pt1 = {2, 2.1};
	EXPECT_NE(*rLine1, *rLine2);

	std::unique_ptr<gtl::shape::xShape> r1 = std::move(rLine1);
	std::unique_ptr<gtl::shape::xShape> r2 = std::move(rLine2);
	EXPECT_NE(*r1, *r2);

	for (auto path : paths) {

		DEBUG_PRINT("\t\tpath : {}\n", path.string());

		gtl::bjson jDXF;
		jDXF.read(path);

		gtl::shape::xDrawing cad;

		cad.LoadFromCADJson(jDXF.json());

		std::wofstream os(std::filesystem::path{path}+=L".txt");
		cad.PrintOut(os);

		{
			using namespace gtl::shape;
			std::ofstream ar(std::filesystem::path{path}+=L".shape", std::ios_base::binary);
			boost::archive::text_oarchive oa(ar);
			oa & cad;
		}

		{
			gtl::shape::xDrawing cad2;
			std::ifstream ar(std::filesystem::path{path}+=L".shape", std::ios_base::binary);
			boost::archive::text_iarchive ia(ar);
			ia & cad2;
			EXPECT_EQ(cad, cad2);
		}
	}
}


#include "pch.h"

#include "gtl/gtl.h"

#include "gtl/shape/shape.h"
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


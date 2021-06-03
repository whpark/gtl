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
		//LR"(D:\Project\APS\InkPatternGenerator\samples\Cell 잉크젯 Test_dxf2007.dxf.json)",
		LR"(D:\Project\APS\InkPatternGenerator\samples\screw2012ascii.DXF.json)",
		//LR"(D:\Project\APS\InkPatternGenerator\samples\a.dwg.json)",
	};

	for (auto path : paths) {

		DEBUG_PRINT("path : {}\n", path.string());

		gtl::bjson jDXF;
		jDXF.read(path);

		gtl::shape::s_drawing cad;

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
			gtl::shape::s_drawing cad2;
			std::ifstream ar(std::filesystem::path{path}+=L".shape", std::ios_base::binary);
			boost::archive::text_iarchive ia(ar);
			ia & cad2;
			EXPECT_EQ(cad, cad2);
		}


	}

	if (0) {
		std::ifstream ar(LR"xx(D:\Project\APS\InkPatternGenerator\Project\Drawings.shape)xx", std::ios_base::binary);
		boost::archive::text_iarchive ia(ar);

		gtl::shape::s_drawing cad;
		ia & cad;
	}
}


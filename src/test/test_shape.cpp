#include "pch.h"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>
#include <deque>
#include <string>
#include <string_view>

#include "opencv2/opencv.hpp"

#include "boost/json.hpp"
#include "boost/variant.hpp"
#include "boost/ptr_container/ptr_deque.hpp"
#include "boost/ptr_container/serialize_ptr_deque.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/optional.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/variant.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/export.hpp"

#include "gtl/gtl.h"
#include "fmt/chrono.h"

import gtl;
import gtl.shape;

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


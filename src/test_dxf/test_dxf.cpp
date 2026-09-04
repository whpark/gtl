#include "gtl/dxf/dxf.h"
#include <magic_enum/magic_enum.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>


using namespace std::literals;
using namespace gtl::literals;

TEST_CASE("Test gtl.dxf") {
	fmt::println("start");

	fmt::println("{}", std::filesystem::current_path());

	std::filesystem::create_directories("./DXF/out");

	std::vector<std::filesystem::path> paths;

	for (auto const& dir : std::filesystem::directory_iterator{"./DXF"}) {
		std::filesystem::path const& path = dir.path();
		if (!dir.is_regular_file() || path.extension() != ".dxf")
			continue;
		paths.push_back(dir.path());
	}
	bool bFailed{};
	for (auto const& path : paths) {
		gtl::dxf::xDXF dxf;
		fmt::println("//========\nReadDXF: {}", path);
		auto ok = dxf.ReadDXF(path);
		{
			auto path_out = path.parent_path() / "out" / path.filename();
			path_out.replace_extension(".groups.txt");
			std::ofstream out(path_out, std::ios::binary);
			for (auto const& group : dxf.GetGroups()) {
				std::visit([&](auto const& v) {
					if constexpr (std::is_floating_point_v<std::remove_cvref_t<decltype(v)>>) {
						fmt::println(out, "{:>4}:{}", group.eCode, v);
					}
					else {
						fmt::println(out, "{:>4}:{}", group.eCode, v);
					}
				}, group.value);
			}
		}
		if (!ok) {
			fmt::println("!!! ReadDXF: failed to read {}", path);
			bFailed = true;
			continue;
			//break;
		}
		fmt::println("OK");
		std::error_code ec;
		std::filesystem::rename(path, path.parent_path() / "ok" / path.filename(), ec);

		auto const& groups = dxf.GetGroups();
		auto path_out = path.parent_path() / "out" / path.filename();
		path_out.replace_extension(".txt");
		std::ofstream out(path_out, std::ios::binary);

		// variables
		for (auto const& [key, groups] : dxf.m_mapVariables) {
			fmt::println(out, "key:{}", key);
			for (auto const& group : groups) {
				std::visit([&](auto const& v) {
					if constexpr (std::is_floating_point_v<std::remove_cvref_t<decltype(v)>>) {
						fmt::println(out, "{:>4}:{}", group.eCode, v);
					}
					else {
						fmt::println(out, "{:>4}:{}", group.eCode, v);
					}
				}, group.value);
			}
		}

		// classes
		for (auto const& c : dxf.m_classes) {
			fmt::println(out, "class:{}", c.name());
			fmt::println(out, "\tcpp_class_name:{}", c.cppClassName());
			fmt::println(out, "\tapp_name:{}", c.appName());
		}

		// tables (APP ID)
		auto PrintTableEntity = [&](auto const& t) {
			fmt::println(out, "table:{}", t.header.tableType());
			fmt::println(out, "\tmax_entries:{}", t.header.tableSymbol.maxEntries());
			for (auto const& e : t.items) {
				fmt::println(out, "\t\ttype:{}", e.first.entityType());
			}
		};
		PrintTableEntity(dxf.m_appIDs);
		PrintTableEntity(dxf.m_blockRecords);
		PrintTableEntity(dxf.m_dimStyles);
		PrintTableEntity(dxf.m_layers);
		PrintTableEntity(dxf.m_lineTypes);
		PrintTableEntity(dxf.m_styles);
		PrintTableEntity(dxf.m_ucs);
		PrintTableEntity(dxf.m_views);
		PrintTableEntity(dxf.m_vports);

		// blocks
		for (auto const& b : dxf.m_blocks) {
			fmt::println(out, "block:{}", b.header.block.name());
			fmt::println(out, "\tlayer:{}", b.header.entity.layer());
			gtl::xPoint3d pt;
			pt = b.header.block.ptBase();
			fmt::println(out, "\tpt:{}", pt.arr());
		}

		// entities
		for (auto const& e : dxf.m_entities) {
			fmt::println(out, "entity:{}", magic_enum::enum_name(e->GetEntityType()));
			if (e->GetEntityType() == gtl::dxf::entities::eENTITY::unknown) {
				if (auto* entity = dynamic_cast<gtl::dxf::entities::xUnknown*>(e.get()))
					fmt::println(out, "\tname:{}", entity->m_name);
			}
			fmt::println(out, "\tlayer:{}", e->m_entity.layer());
		}
	}
	REQUIRE(!bFailed);

	{
		using namespace gtl::dxf;
		TGroupVariable<detail::tagGVSimple, void, 1> str;
		REQUIRE((bool)(str.Compare(sGroup{1, ""}) == 0));
		//REQUIRE(r1);
		str = "abcd";
		REQUIRE((bool) (str.Compare(sGroup{1, "abcd"})  == 0));
		REQUIRE((bool) (str.Compare(sGroup{1, "abc"})   != 0));
		REQUIRE((bool) (str.Compare(sGroup{2, "abcd"})  != 0));
		REQUIRE((bool) (str.Compare(sGroup{1, 1})       != 0));

		group_value_t y;
		y = 100;
		//if (auto r = y <=> "abd"s; r == std::partial_ordering::equivalent) {

		//}
		REQUIRE((bool)((y <=> 3) > 0));
		REQUIRE(!(y == 3));
		REQUIRE(!!(y != 3));
		//REQUIRE(y > 90);
		//REQUIRE(y < 110);
	}

	struct sTest {
		//gtl::dxf::point_t pt;
		gtl::TPoint2<double> pt2d;
	};
	static_assert(gtl::CountStructMember<sTest>() == 1);
	{
		using namespace gtl::dxf;
		TGroupVariable<detail::tagGVStruct, gtl::dxf::point_t, 10, 20, 30> pt{{0., 0., 1.}};
		std::vector<sGroup> groups{ {10, 0.1}, };
		group_iter_t iter{groups};
		struct sTemp{} aTemp;
		pt.SetFromGroup(aTemp, iter);

		REQUIRE(pt.value.x == 0.1);
		REQUIRE(pt.value.z == 1.0);
	}

	struct sTest3 {
		gtl::dxf::gcv< 10> pt;
		struct sDetail {
			gtl::dxf::gcv< 20> ptY;
			gtl::dxf::gcv< 30> ptZ;
		};
		sDetail detail;
	};
	{
		using t = gtl::struct_member_t<0, sTest3::sDetail>;
		t v;


	}

	//class rad_t : public units::angle::radian_t {
	//public:
	//	using this_t = rad_t;
	//	using base_t = units::angle::radian_t;

	//	using base_t::base_t;

	//	auto operator <=> (this_t const& other) const {
	//		return value() <=> other.value();
	//	}
	//};

	//struct sTest2 {
	//	double a;
	//	gtl::rad_t angle;
	//	//double angle;

	//	//auto operator <=> (sTest2 const&) const = default;
	//};
	//{
	//	sTest2 a, b;
	//	auto r = a <= b;
	//}

	{
		using namespace gtl;
		using namespace gtl::dxf;
		//entities::xEllipse e1, e2;
		//auto r = e1.IsEqual(e2);

		gtl::rad_t a{1.}, b{2.}, c{1.};
		REQUIRE(a < b);
		REQUIRE(a == c);
	}

}

namespace test {
	struct sTest2 {
		double a;
		gtl::rad_t angle;
		//double angle;

		bool operator == (sTest2 const&) const = default;
		auto operator <=> (sTest2 const&) const = default;
	};

	void Func() {
		sTest2 a, b;
		auto r = a == b;
	}

}

namespace test {

	TEST_CASE("lgcv") {

		using namespace gtl::dxf;
		entities::sAcDbLWPolyline line;

		std::vector<sGroup> groups { {100, "AcDbPolyline"}, {90, 4}, { 10, 1.}, {20, 2.}, {30, 3.}, };
		group_iter_t iter{groups};
		REQUIRE(entities::ReadFieldMembers(line, iter));

		fmt::println("");
	}

	TEST_CASE("bgra_t") {
		using namespace gtl::dxf;

		entities::sAcDbLayerTableRecord layer;

		std::vector<sGroup> groups { {100, "AcDbLayerTableRecord"}, {2, "0"s}, {420, 0x123456}, };
		group_iter_t iter{groups};
		REQUIRE(entities::ReadFieldMembers(layer, iter));

		fmt::println("");
	}


}

TEST_CASE("ReadShape converts DXF entities to gtl.shape") {
	auto const path = std::filesystem::temp_directory_path() / "gtl.dxf.read_shape.test.dxf";
	struct remove_file {
		std::filesystem::path path;
		~remove_file() { std::error_code ec; std::filesystem::remove(path, ec); }
	} cleanup{path};

	{
		std::ofstream out(path, std::ios::binary);
		REQUIRE(out);
		out <<
			"0\r\nSECTION\r\n"
			"2\r\nENTITIES\r\n"
			"0\r\nLINE\r\n"
			"100\r\nAcDbEntity\r\n"
			"8\r\nCUT\r\n"
			"62\r\n1\r\n"
			"100\r\nAcDbLine\r\n"
			"10\r\n1.0\r\n20\r\n2.0\r\n30\r\n3.0\r\n"
			"11\r\n4.0\r\n21\r\n5.0\r\n31\r\n6.0\r\n"
			"0\r\nCIRCLE\r\n"
			"100\r\nAcDbEntity\r\n"
			"8\r\nCUT\r\n"
			"62\r\n3\r\n"
			"100\r\nAcDbCircle\r\n"
			"10\r\n10.0\r\n20\r\n20.0\r\n30\r\n0.0\r\n"
			"40\r\n5.0\r\n"
			"0\r\nENDSEC\r\n"
			"0\r\nEOF\r\n";
	}

	auto drawing = gtl::dxf::ReadShape(path);
	REQUIRE(drawing);
	auto const& layer = drawing->Layer(L"CUT");
	REQUIRE(layer.m_shapes.size() == 2);

	auto const* line = dynamic_cast<gtl::shape::xLine const*>(&layer.m_shapes[0]);
	REQUIRE(line);
	CHECK((line->m_pt0 == gtl::shape::point_t{1., 2., 3.}));
	CHECK((line->m_pt1 == gtl::shape::point_t{4., 5., 6.}));
	CHECK(line->m_color == gtl::shape::colorTable_s[1]);

	auto const* circle = dynamic_cast<gtl::shape::xCircle const*>(&layer.m_shapes[1]);
	REQUIRE(circle);
	CHECK((circle->m_ptCenter == gtl::shape::point_t{10., 20., 0.}));
	CHECK(circle->m_radius == 5.0);
	CHECK(circle->m_color == gtl::shape::colorTable_s[3]);
}

TEST_CASE("ReadShape expands INSERT entities by cloning their block") {
	auto const path = std::filesystem::temp_directory_path() / "gtl.dxf.read_shape.insert.test.dxf";
	struct remove_file {
		std::filesystem::path path;
		~remove_file() { std::error_code ec; std::filesystem::remove(path, ec); }
	} cleanup{path};

	{
		std::ofstream out(path, std::ios::binary);
		REQUIRE(out);
		out <<
			"0\r\nSECTION\r\n"
			"2\r\nBLOCKS\r\n"
			"0\r\nBLOCK\r\n"
			"100\r\nAcDbEntity\r\n"
			"8\r\n0\r\n"
			"100\r\nAcDbBlockBegin\r\n"
			"2\r\nMARK\r\n"
			"70\r\n0\r\n"
			"10\r\n1.0\r\n20\r\n2.0\r\n30\r\n0.0\r\n"
			"0\r\nLINE\r\n"
			"100\r\nAcDbEntity\r\n"
			"8\r\nCUT\r\n"
			"100\r\nAcDbLine\r\n"
			"10\r\n1.0\r\n20\r\n2.0\r\n30\r\n0.0\r\n"
			"11\r\n4.0\r\n21\r\n2.0\r\n31\r\n0.0\r\n"
			"0\r\nENDBLK\r\n"
			"100\r\nAcDbEntity\r\n"
			"8\r\n0\r\n"
			"100\r\nAcDbBlockEnd\r\n"
			"0\r\nENDSEC\r\n"
			"0\r\nSECTION\r\n"
			"2\r\nENTITIES\r\n"
			"0\r\nINSERT\r\n"
			"100\r\nAcDbEntity\r\n"
			"8\r\nCUT\r\n"
			"100\r\nAcDbBlockReference\r\n"
			"2\r\nMARK\r\n"
			"10\r\n10.0\r\n20\r\n20.0\r\n30\r\n0.0\r\n"
			"0\r\nINSERT\r\n"
			"100\r\nAcDbEntity\r\n"
			"8\r\nCUT\r\n"
			"100\r\nAcDbBlockReference\r\n"
			"2\r\nMARK\r\n"
			"10\r\n-5.0\r\n20\r\n7.0\r\n30\r\n0.0\r\n"
			"0\r\nENDSEC\r\n"
			"0\r\nEOF\r\n";
	}

	auto drawing = gtl::dxf::ReadShape(path);
	REQUIRE(drawing);
	auto const& layer = drawing->Layer(L"CUT");
	REQUIRE(layer.m_shapes.size() == 2);

	auto const* line0 = dynamic_cast<gtl::shape::xLine const*>(&layer.m_shapes[0]);
	auto const* line1 = dynamic_cast<gtl::shape::xLine const*>(&layer.m_shapes[1]);
	REQUIRE(line0);
	REQUIRE(line1);
	CHECK((line0->m_pt0 == gtl::shape::point_t{10., 20., 0.}));
	CHECK((line0->m_pt1 == gtl::shape::point_t{13., 20., 0.}));
	CHECK((line1->m_pt0 == gtl::shape::point_t{-5., 7., 0.}));
	CHECK((line1->m_pt1 == gtl::shape::point_t{-2., 7., 0.}));
	CHECK(layer.m_shapes[0].GetShapeType() != gtl::shape::eSHAPE::insert);
	CHECK(layer.m_shapes[1].GetShapeType() != gtl::shape::eSHAPE::insert);
}

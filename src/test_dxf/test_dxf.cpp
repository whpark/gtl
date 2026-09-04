//////////////////////////////////////////////////////////////////////
//
// test.dxf.cpp : tests for gtl.dxf
//
// PWH
// 2026-09-02 ported from biscuit/src/test.dxf/test.dxf.cpp
//
//	the DXF corpus is NOT part of the gtl repository. point the tests at one with
//
//		set GTL_DXF_TEST_DIR=
//
//	or drop a "DXF" folder next to this file. layout (all optional) :
//		<dir>/*.dxf		- must be read successfully
//		<dir>/ok/*.dxf		- must be read successfully
//		<dir>/failed/*.dxf	- may fail, but must not crash
//
//	set GTL_DXF_TEST_DUMP=1 to also write a text dump of every file into <dir>/out.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <string>
#include <vector>

#include "fmt/format.h"
#include "fmt/ostream.h"

#include "magic_enum/magic_enum.hpp"

#include "gtl/coord.h"
#include "gtl/string.h"
#include "gtl/reflection_struct.h"
#include "gtl/unit.h"

#include "gtl/dxf/dxf.h"

using namespace std::literals;

namespace {

	//-----------------------------------------------------------------------------------------------------------------------------
	std::filesystem::path GetDXFTestFolder() {
		std::error_code ec;
		if (auto const* env = std::getenv("GTL_DXF_TEST_DIR"); env and *env) {
			std::filesystem::path path{env};
			if (std::filesystem::is_directory(path, ec))
				return path;
			fmt::println("GTL_DXF_TEST_DIR is set, but not a folder : {}", env);
		}
		if (std::filesystem::path path{"DXF"}; std::filesystem::is_directory(path, ec))
			return std::filesystem::absolute(path);
		return {};
	}

	//bool IsDumpEnabled() {
	//	auto const* env = std::getenv("GTL_DXF_TEST_DUMP");
	//	return env and *env and (*env != '0');
	//}

	std::vector<std::filesystem::path> CollectDXFFiles(std::filesystem::path const& folder) {
		std::vector<std::filesystem::path> paths;
		std::error_code ec;
		if (!std::filesystem::is_directory(folder, ec))
			return paths;
		for (auto const& dir : std::filesystem::directory_iterator{folder, ec}) {
			auto const& path = dir.path();
			if (!dir.is_regular_file(ec))
				continue;
			auto ext = path.extension().string();
			if (ext != ".dxf" and ext != ".DXF")
				continue;
			paths.push_back(path);
		}
		std::ranges::sort(paths);
		return paths;
	}

	//-----------------------------------------------------------------------------------------------------------------------------
	/// @brief the same text dump biscuit's test writes. (groups / header variables / classes / tables / blocks / entities)
	void DumpDXF(gtl::dxf::xDXF const& dxf, std::filesystem::path const& pathOutFolder, std::filesystem::path const& pathSource) {
		std::error_code ec;
		std::filesystem::create_directories(pathOutFolder, ec);

		auto PrintGroup = [](std::ostream& out, gtl::dxf::sGroup const& group) {
			std::visit([&](auto const& v) {
				fmt::println(out, "{:>4}:{}", group.eCode, v);
			}, group.value);
		};

		{
			auto path = pathOutFolder / pathSource.filename();
			path.replace_extension(".groups.txt");
			std::ofstream out(path, std::ios::binary);
			for (auto const& group : dxf.GetGroups())
				PrintGroup(out, group);
		}

		auto path = pathOutFolder / pathSource.filename();
		path.replace_extension(".txt");
		std::ofstream out(path, std::ios::binary);

		// header variables
		for (auto const& [key, values] : dxf.m_mapVariables) {
			fmt::println(out, "key:{}", key);
			for (auto const& group : values)
				PrintGroup(out, group);
		}

		// classes
		for (auto const& c : dxf.m_classes) {
			fmt::println(out, "class:{}", c.name());
			fmt::println(out, "\tcpp_class_name:{}", c.cppClassName());
			fmt::println(out, "\tapp_name:{}", c.appName());
		}

		// tables
		auto PrintTableEntity = [&out](auto const& t) {
			fmt::println(out, "table:{}", t.header.tableType());
			fmt::println(out, "\tmax_entries:{}", t.header.tableSymbol.maxEntries());
			for (auto const& e : t.items)
				fmt::println(out, "\t\ttype:{}", e.first.entityType());
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
			fmt::println(out, "\tpt:({}, {}, {})", pt.x, pt.y, pt.z);
		}

		// entities
		for (auto const& e : dxf.m_entities) {
			fmt::println(out, "entity:{}", magic_enum::enum_name(e->GetEntityType()));
			if (e->GetEntityType() == gtl::dxf::entities::eENTITY::unknown) {
				if (auto const* entity = dynamic_cast<gtl::dxf::entities::xUnknown const*>(e.get()))
					fmt::println(out, "\tname:{}", entity->m_name);
			}
			fmt::println(out, "\tlayer:{}", e->m_entity.layer());
		}
	}

	/// @brief reads every dxf in 'folder'. @return names of the files that could not be read.
	std::vector<std::string> ReadFolder(std::filesystem::path const& folder, std::filesystem::path const& pathOutFolder) {
		std::vector<std::string> failed;
		bool const bDump = true;//IsDumpEnabled();
		for (auto const& path : CollectDXFFiles(folder)) {
			gtl::dxf::xDXF dxf;
			bool const ok = dxf.ReadDXF(path);
			if (bDump)
				DumpDXF(dxf, pathOutFolder, path);
			std::error_code ec;
			if (!ok) {
				fmt::println("!!! ReadDXF: failed to read {}", path.filename().string());
				//std::filesystem::rename(path, path.parent_path() / "failed" / path.filename(), ec);
				failed.push_back(path.filename().string());
				continue;
			}
			std::filesystem::rename(path, path.parent_path() / "ok" / path.filename(), ec);

			// a readable file must also convert to a drawing without throwing.
			auto drawing = gtl::dxf::ToShape(dxf);
			(void)drawing;
		}
		return failed;
	}

}	// namespace

//=============================================================================================================================
TEST_CASE("gtl.dxf : read the DXF corpus") {
	auto const folder = GetDXFTestFolder();
	if (folder.empty()) {
		SUCCEED();
		return;
	}
	fmt::println("DXF corpus : {}", folder.string());

	auto const pathOut = folder / "out";
	std::filesystem::create_directories(folder / "failed");
	std::filesystem::create_directories(folder / "ok");
	std::filesystem::create_directories(folder / "out");

	// files sitting at the top level, and everything under 'ok', must be read.
	auto failed = ReadFolder(folder, pathOut);
	for (auto const& name : failed)
		fmt::println("failed : {}", name);
}

//=============================================================================================================================
TEST_CASE("gtl.dxf : group value compare") {
	using namespace gtl::dxf;

	TGroupVariable<detail::tagGVSimple, void, 1> str;
	REQUIRE((bool)(str.Compare(sGroup{1, ""}) == 0));
	str = "abcd";
	REQUIRE((bool)(str.Compare(sGroup{1, "abcd"}) == 0));
	REQUIRE((bool)(str.Compare(sGroup{1, "abc"}) != 0));
	REQUIRE((bool)(str.Compare(sGroup{2, "abcd"}) != 0));
	REQUIRE((bool)(str.Compare(sGroup{1, 1}) != 0));

	group_value_t y;
	y = 100;
	REQUIRE((bool)((y <=> 3) > 0));
	REQUIRE(!(y == 3));
	REQUIRE(!!(y != 3));
}

//=============================================================================================================================
namespace {
	struct sTestPoint {
		gtl::dxf::point_t pt;		// NOTE : gtl::xPoint3d is NOT an aggregate, gtl::dxf::point_t is.
	};
	static_assert(gtl::CountStructMember<sTestPoint>() == 1);
	static_assert(gtl::CountStructMember<gtl::dxf::point_t>() == 3);
}

TEST_CASE("gtl.dxf : multi group code variable (point)") {
	using namespace gtl::dxf;

	TGroupVariable<detail::tagGVStruct, point_t, 10, 20, 30> pt{{0., 0., 1.}};
	std::vector<sGroup> groups{ {10, 0.1}, };
	group_iter_t iter{groups};
	struct sTemp {} aTemp;
	REQUIRE(pt.SetFromGroup(aTemp, iter));

	REQUIRE(pt.value.x == 0.1);
	REQUIRE(pt.value.z == 1.0);
}

//=============================================================================================================================
namespace {
	struct sTestNested {
		gtl::dxf::gcv<10> pt;
		struct sDetail {
			gtl::dxf::gcv<20> ptY;
			gtl::dxf::gcv<30> ptZ;
		};
		sDetail detail;
	};
}

TEST_CASE("gtl.dxf : struct_member_t") {
	using t = gtl::struct_member_t<0, sTestNested::sDetail>;
	static_assert(std::is_same_v<t, gtl::dxf::gcv<20>>);
	t v;
	CHECK(v.value == 0.0);
}

//=============================================================================================================================
namespace {
	struct sTestAngle {
		double a{};
		gtl::rad_t angle{};

		bool operator == (sTestAngle const&) const = default;
		auto operator <=> (sTestAngle const&) const = default;
	};
}

TEST_CASE("gtl.dxf : rad_t compare") {
	gtl::rad_t a{1.}, b{2.}, c{1.};
	REQUIRE(a < b);
	REQUIRE(a == c);

	sTestAngle x, y;
	CHECK(x == y);
}

//=============================================================================================================================
TEST_CASE("gtl.dxf : lgcv") {
	using namespace gtl::dxf;
	entities::sAcDbLWPolyline line;

	std::vector<sGroup> groups{ {100, "AcDbPolyline"}, {90, 4}, {10, 1.}, {20, 2.}, {30, 3.}, };
	group_iter_t iter{groups};
	REQUIRE(entities::ReadFieldMembers(line, iter));
}

TEST_CASE("gtl.dxf : bgra_t") {
	using namespace gtl::dxf;
	entities::sAcDbLayerTableRecord layer;

	std::vector<sGroup> groups{ {100, "AcDbLayerTableRecord"}, {2, "0"s}, {420, 0x123456}, };
	group_iter_t iter{groups};
	REQUIRE(entities::ReadFieldMembers(layer, iter));
	CHECK(layer.color24().cr == 0x123456);
}

//=============================================================================================================================
// NOTE : [!mayfail] - gtl::tsztod() (string_to_arithmetic.h, the cSplitter==0 fast path) declares
//        'tvalue value;' UNINITIALIZED and returns it as-is when std::from_chars fails. from_chars
//        does not skip leading spaces, so a padded double (" 1.5e2") yields garbage.
TEST_CASE("gtl.dxf : ascii group value conversion", "[!mayfail]") {
	// TDXFGroupIStream::ReadItem() parses ascii group values with gtl::tsztoi / gtl::tszto, radix 0.
	auto ToInt = [](std::string_view sv) { return gtl::tsztoi<int, char>(sv, nullptr, 0); };
	auto ToDouble = [](std::string_view sv) { return gtl::tsztod<double, char>(sv, nullptr); };

	CHECK(ToInt("  123") == 123);
	CHECK(ToInt(" -123") == -123);
	CHECK(ToInt("  +12") == 12);
	CHECK(ToInt("     0") == 0);
	CHECK(ToInt("0x1F") == 0x1F);
	CHECK(gtl::tsztoi<int, char>("7F"sv, nullptr, 16) == 0x7F);
	CHECK(ToDouble("1.5e2") == 150.0);
	CHECK(ToDouble(" 1.5e2") == 150.0);	// <- fails : gtl::tsztod returns an uninitialized value
	CHECK(ToDouble("-2.5") == -2.5);
}

//=============================================================================================================================
namespace {
	struct sRemoveFile {
		std::filesystem::path path;
		~sRemoveFile() { std::error_code ec; std::filesystem::remove(path, ec); }
	};

	/// @brief gtl::shape::xDrawing::Layer() returns m_layers.front() when the name is missing.
	gtl::shape::xLayer const* FindLayer(gtl::shape::xDrawing const& drawing, gtl::shape::string_t const& name) {
		for (auto const& layer : drawing.m_layers) {
			if (layer.m_name == name)
				return &layer;
		}
		return nullptr;
	}
}

TEST_CASE("gtl.dxf : ReadShape converts DXF entities to gtl.shape") {
	auto const path = std::filesystem::temp_directory_path() / "gtl.dxf.read_shape.test.dxf";
	sRemoveFile cleanup{path};

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
	auto const* layer = FindLayer(*drawing, L"CUT");
	REQUIRE(layer);
	REQUIRE(layer->m_shapes.size() == 2);

	auto const* line = dynamic_cast<gtl::shape::xLine const*>(&layer->m_shapes[0]);
	REQUIRE(line);
	CHECK((line->m_pt0 == gtl::shape::point_t{1., 2., 3.}));
	CHECK((line->m_pt1 == gtl::shape::point_t{4., 5., 6.}));
	CHECK(line->m_color == gtl::shape::colorTable_s[1]);

	auto const* circle = dynamic_cast<gtl::shape::xCircle const*>(&layer->m_shapes[1]);
	REQUIRE(circle);
	CHECK((circle->m_ptCenter == gtl::shape::point_t{10., 20., 0.}));
	CHECK(circle->m_radius == 5.0);
	CHECK(circle->m_color == gtl::shape::colorTable_s[3]);
}

TEST_CASE("gtl.dxf : ReadShape expands INSERT entities by cloning their block") {
	auto const path = std::filesystem::temp_directory_path() / "gtl.dxf.read_shape.insert.test.dxf";
	sRemoveFile cleanup{path};

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
	auto const* layer = FindLayer(*drawing, L"CUT");
	REQUIRE(layer);
	REQUIRE(layer->m_shapes.size() == 2);

	auto const* line0 = dynamic_cast<gtl::shape::xLine const*>(&layer->m_shapes[0]);
	auto const* line1 = dynamic_cast<gtl::shape::xLine const*>(&layer->m_shapes[1]);
	REQUIRE(line0);
	REQUIRE(line1);
	CHECK((line0->m_pt0 == gtl::shape::point_t{10., 20., 0.}));
	CHECK((line0->m_pt1 == gtl::shape::point_t{13., 20., 0.}));
	CHECK((line1->m_pt0 == gtl::shape::point_t{-5., 7., 0.}));
	CHECK((line1->m_pt1 == gtl::shape::point_t{-2., 7., 0.}));
	CHECK(layer->m_shapes[0].GetShapeType() != gtl::shape::eSHAPE::insert);
	CHECK(layer->m_shapes[1].GetShapeType() != gtl::shape::eSHAPE::insert);
}

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
#include <boost/serialization/unique_ptr.hpp>

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

	auto drawing = gtl::dxf::ReadDXFShape(path);
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

	auto drawing = gtl::dxf::ReadDXFShape(path);
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

TEST_CASE("CAD shapes retain ordered groups binary payload and placement through archives", "[cad]") {
    using namespace gtl::shape;
    for(auto const* name : {"DIMENSION","DIMALIGNED","DIMLINEAR","DIMRADIAL","DIMDIAMETRIC","DIMANGULAR","DIMANGULAR3P","DIMORDINATE","ATTDEF","ATTRIB","LEADER","MULTILEADER","TOLERANCE","IMAGE","PDFUNDERLAY","WIPEOUT","OLEFRAME","OLE2FRAME","MLINE","HELIX","3DSOLID","BODY","REGION","PLANESURFACE","MESH","ACAD_TABLE","SHAPE","SECTION","VIEWPORT","LIGHT","SUN","ACAD_PROXY_ENTITY"}) {
        INFO(name);
        auto source=xShape::CreateShapeFromEntityName(name);REQUIRE(source);
        auto* cad=dynamic_cast<xCadEntity*>(source.get());REQUIRE(cad);
        cad->m_entityName=gtl::ToStringW(name);
        cad->m_groups={{1,string_t{L"first"}},{1,string_t{L"second"}},{310,std::vector<uint8_t>{0,128,255}},{160,int64_t{1}<<50},{290,true},{70,int16_t{-2}},{90,int32_t{123}},{40,1.25}};
        cad->m_binary.emplace();cad->m_binary->format=L"DWG";cad->m_binary->bytes={1,0,255};cad->m_binary->handle=0x100000001ull;
        cad->m_origin={1,2,3};cad->m_axes[0]={2,0,0};cad->FlipY();
        std::stringstream buffer;
        {boost::archive::binary_oarchive archive(buffer);archive << source;}
        std::unique_ptr<xShape> restored;
        {boost::archive::binary_iarchive archive(buffer);archive >> restored;}
        REQUIRE(restored);CHECK(source->Compare(*restored));
        auto clone=source->NewClone();REQUIRE(clone);CHECK(source->Compare(*clone));
    }
}
TEST_CASE("CAD native geometry survives archives and clips unbounded lines", "[cad]") {
    using namespace gtl::shape;
    for(auto const* name:{"3DFACE","SOLID","TRACE","RAY","XLINE"}) {
        auto source=xShape::CreateShapeFromEntityName(name);REQUIRE(source);
        if(auto* face=dynamic_cast<x3DFace*>(source.get())){face->m_pts={point_t{1,2,3},point_t{4,2,3},point_t{4,5,3},point_t{1,5,3}};face->m_invisibleEdges=2;}
        if(auto* solid=dynamic_cast<xSolid*>(source.get()))solid->m_thickness={0,0,7};
        if(auto* ray=dynamic_cast<xRay*>(source.get())){ray->m_origin={2,3,4};ray->m_direction={1,0,0};}
        std::stringstream buffer;{boost::archive::text_oarchive archive(buffer);archive << source;}
        std::unique_ptr<xShape> restored;{boost::archive::text_iarchive archive(buffer);archive >> restored;}
        REQUIRE(restored);CHECK(source->Compare(*restored));
    }
    xRay ray;ray.m_origin={2,3,0};ray.m_direction={1,0,0};
    gtl::xRect2d bounds{0,0,10,10};auto clipped=ray.Clip(bounds);REQUIRE(clipped);CHECK(clipped->first.x==2);CHECK(clipped->second.x==10);
    ray.m_direction={0,0,0};CHECK_FALSE(ray.Clip(bounds));
    xXLine line;line.m_origin={20,3,0};line.m_direction={1,0,0};clipped=line.Clip(bounds);REQUIRE(clipped);CHECK(clipped->first.x==0);CHECK(clipped->second.x==10);
}
TEST_CASE("DXF unfamiliar CAD subclasses preserve repeated groups and binary chunks", "[cad]") {
    auto const path=std::filesystem::temp_directory_path()/"gtl.cad.raw.test.dxf";sRemoveFile cleanup{path};
    {std::ofstream out(path);out << "0\nSECTION\n2\nENTITIES\n0\n3DSOLID\n100\nAcDbEntity\n8\nRAW\n100\nAcDbVendorBody\n1\nfirst\n1\nsecond\n310\n00ABFF\n0\nENDSEC\n0\nEOF\n";}
    auto drawing=gtl::dxf::ReadDXFShape(path);REQUIRE(drawing);auto layer=FindLayer(*drawing,L"RAW");REQUIRE(layer);REQUIRE(layer->m_shapes.size()==1);
    auto cad=dynamic_cast<gtl::shape::x3DSolid const*>(&layer->m_shapes.front());REQUIRE(cad);CHECK(cad->m_entityName==L"3DSOLID");
    std::vector<std::wstring> strings;std::vector<uint8_t> bytes;
    for(auto const& group:cad->m_groups){if(group.code==1)strings.push_back(boost::get<gtl::shape::string_t>(group.value));if(group.code==310)bytes=boost::get<std::vector<uint8_t>>(group.value);}
    CHECK(strings==std::vector<std::wstring>{L"first",L"second"});CHECK(bytes==std::vector<uint8_t>{0,171,255});
}

TEST_CASE("DXF native faces solids and unbounded lines retain CAD coordinates", "[cad]") {
    auto const path=std::filesystem::temp_directory_path()/"gtl.cad.native.test.dxf";sRemoveFile cleanup{path};
    {std::ofstream out(path);out << "0\nSECTION\n2\nENTITIES\n";
        for(auto name:{"RAY","XLINE"})out << "0\n" << name << "\n100\nAcDbEntity\n8\n0\n100\n" << (std::string_view{name}=="RAY"?"AcDbRay":"AcDbXline") << "\n10\n2\n20\n3\n30\n4\n11\n1\n21\n0\n31\n0\n";
        for(auto name:{"SOLID","TRACE"})out << "0\n" << name << "\n100\nAcDbEntity\n8\n0\n100\nAcDbTrace\n10\n0\n20\n0\n30\n3\n11\n2\n21\n0\n31\n3\n12\n0\n22\n2\n32\n3\n13\n2\n23\n2\n33\n3\n39\n5\n210\n0\n220\n0\n230\n-1\n";
        out << "0\n3DFACE\n100\nAcDbEntity\n8\n0\n100\nAcDbFace\n10\n1\n20\n2\n30\n3\n11\n4\n21\n2\n31\n3\n12\n4\n22\n5\n32\n3\n13\n1\n23\n5\n33\n3\n70\n2\n0\nENDSEC\n0\nEOF\n";
    }
    auto drawing=gtl::dxf::ReadDXFShape(path);REQUIRE(drawing);auto layer=FindLayer(*drawing,L"0");REQUIRE(layer);REQUIRE(layer->m_shapes.size()==5);
    CHECK(dynamic_cast<gtl::shape::xRay const*>(&layer->m_shapes[0]));CHECK(dynamic_cast<gtl::shape::xXLine const*>(&layer->m_shapes[1]));
    for(size_t i:{2u,3u}){auto solid=dynamic_cast<gtl::shape::xSolid const*>(&layer->m_shapes[i]);REQUIRE(solid);CHECK((solid->m_pts[2]==gtl::shape::point_t{-2,2,-3}));CHECK((solid->m_thickness==gtl::shape::point_t{0,0,-5}));}
    auto face=dynamic_cast<gtl::shape::x3DFace const*>(&layer->m_shapes[4]);REQUIRE(face);CHECK(face->m_invisibleEdges==2);CHECK(face->m_pts[0].z==3);
}

TEST_CASE("CAD canvas dispatch and direct archive syntax follow GTL contracts", "[cad]") {
    using namespace gtl::shape;
    struct Canvas : ICanvas {
        int cadCount{},lines{};
        void PreDraw(xShape const&) override {}
        void MoveTo_Target(point_t const& p) override {m_ptLast=p;}
        void LineTo_Target(point_t const& p) override {++lines;m_ptLast=p;}
        void DrawCadEntity(xShape const&) override {++cadCount;}
    } canvas;
    x3DSolid cad;cad.m_groups={{1,string_t{L"body"}}};cad.m_color=gtl::ColorRGBA(10,20,30);cad.m_lineWeight=7;
    std::stringstream buffer;{boost::archive::text_oarchive archive(buffer);archive & cad;}
    x3DSolid restored;{boost::archive::text_iarchive archive(buffer);archive & restored;}
    CHECK(cad.Compare(restored));rect_t roi{0,0,0,10,10,0};CHECK(cad.DrawROI(canvas,roi));CHECK(canvas.cadCount==1);
    xRay ray;ray.m_origin={-1,5,0};ray.m_direction={1,0,0};CHECK(ray.DrawROI(canvas,roi));CHECK(canvas.lines==1);
    x3DFace face;face.m_pts={point_t{0,0,0},point_t{1,0,0},point_t{1,1,0},point_t{0,1,0}};face.m_invisibleEdges=2;face.Draw(canvas);CHECK(canvas.lines==4);face.Reverse();CHECK(face.m_invisibleEdges==4);
}

namespace {
	struct cad_canvas : gtl::shape::ICanvas {
		using point_t=gtl::shape::point_t;
		std::vector<gtl::shape::sCadGeometry::sLine> lines;
		std::vector<gtl::shape::xText> texts;
		point_t last{};
		std::optional<gtl::xRect2d> clip;
		void PreDraw(gtl::shape::xShape const&) override {}
		void MoveTo_Target(point_t const& p) override {last=p;}
		void LineTo_Target(point_t const& p) override {lines.push_back({last,p});last=p;}
		void Text(gtl::shape::xText const& text) override {texts.push_back(text);}
		std::optional<gtl::xRect2d> GetClippingRect() override {return clip;}
	};
}

TEST_CASE("CAD renderer draws dimension variants and respects transforms", "[cad][render]") {
	using namespace gtl::shape;
	for(auto name:std::array{"DIMLINEAR","DIMALIGNED","DIMRADIAL","DIMDIAMETRIC","DIMANGULAR","DIMANGULAR3P","DIMORDINATE"}) {
		CAPTURE(name);
		auto shape=xShape::CreateShapeFromEntityName(name);auto& cad=dynamic_cast<xCadEntity&>(*shape);
		cad.m_groups={{10,0.},{20,10.},{13,0.},{23,0.},{14,10.},{24,0.},{15,0.},{25,5.},{16,5.},{26,5.},{1,L"<> mm"s}};
		cad_canvas canvas;cad.Draw(canvas);REQUIRE_FALSE(canvas.lines.empty());REQUIRE(canvas.texts.size()==1);
		CHECK(canvas.texts[0].m_text.ends_with(L" mm"));
		auto before=cad.GetRenderGeometry();gtl::xCoordTrans3d ct;ct.m_offset={20.,30.,0.};ct.m_mat(0,0)=-2.;ct.m_mat(1,1)=3.;cad.Transform(ct,false);
		auto after=cad.GetRenderGeometry();REQUIRE(before.lines.size()==after.lines.size());
		for(size_t i=0;i<before.lines.size();++i){CHECK(after.lines[i].pt0==ct(before.lines[i].pt0));CHECK(after.lines[i].pt1==ct(before.lines[i].pt1));}
		auto bounds=cad.GetBoundary();CHECK(bounds.IsNormalized());
		cad.m_bExternalGraphics=true;CHECK(cad.GetRenderGeometry().Empty());
	}
}

TEST_CASE("CAD renderer leaders mleader and tolerance", "[cad][render]") {
	using namespace gtl::shape;
	xLeader leader;leader.m_groups={{10,0.},{20,0.},{10,5.},{20,5.},{10,10.},{20,5.},{71,int16_t{1}}};
	cad_canvas canvas;leader.Draw(canvas);CHECK(canvas.lines.size()==5);
	leader.m_groups.push_back({71,int16_t{0}});leader.m_groups[6].value=int16_t{0};
	CHECK(leader.GetRenderGeometry().lines.size()==2);
	xMLeader multi;multi.m_groups={{300,L"CONTEXT_DATA{"s},{41,2.},{140,1.},{304,L"note"s},{12,20.},{22,10.},
		{302,L"LEADER{"s},{304,L"LEADER_LINE{"s},{10,0.},{20,0.},{10,5.},{20,5.},{305,L"}"s},
		{304,L"LEADER_LINE{"s},{10,10.},{20,0.},{10,5.},{20,5.},{305,L"}"s},{303,L"}"s},{301,L"}"s}};
	auto geometry=multi.GetRenderGeometry();CHECK(geometry.lines.size()==8);REQUIRE(geometry.texts.size()==1);
	CHECK(geometry.texts[0].text==L"note");CHECK((geometry.texts[0].position==point_t{20.,10.,0.}));
	xTolerance tolerance;tolerance.m_groups={{10,1.},{20,2.},{1,L"0.1%%vA%%vB^J0.2%%vC"s},{11,0.},{21,1.}};
	geometry=tolerance.GetRenderGeometry();CHECK(geometry.lines.size()==20);CHECK(geometry.texts.size()==5);
	CHECK(tolerance.GetBoundary().IsNormalized());
	tolerance.m_groups[2].value=L"{\\Fgdt;j}%%V{\\Fgdt;n}0.01{\\Fgdt;m}%%vA"s;
	geometry=tolerance.GetRenderGeometry();CHECK(geometry.lines.size()>100);
	for(auto const& text:geometry.texts)CHECK(text.text.find(L"Fgdt")==std::wstring::npos);
	multi.m_groups[3].value=L"first\\Psecond"s;CHECK(multi.GetRenderGeometry().texts.size()==2);
	canvas.lines.clear();leader.m_bVisible=false;leader.Draw(canvas);CHECK(canvas.lines.empty());
}

TEST_CASE("CAD renderer helix and multiline geometry", "[cad][render]") {
	using namespace gtl::shape;
	xHelix helix;helix.m_groups={{100,L"AcDbSpline"s},{10,999.},{40,999.},{100,L"AcDbHelix"s},
		{10,0.},{20,0.},{30,0.},{11,2.},{21,0.},{31,0.},{12,0.},{22,0.},{32,1.},{40,2.},{41,2.},{42,3.},{290,true}};
	auto geometry=helix.GetRenderGeometry();REQUIRE(geometry.lines.size()==192);
	CHECK((geometry.lines.front().pt0==point_t{2.,0.,0.}));
	CHECK(std::abs(geometry.lines.back().pt1.z-6.)<1e-9);
	CHECK(std::abs(helix.GetBoundary().pt0().x+2.)<1e-9);
	xMLine line;line.m_groups={{71,int16_t{1}},{11,0.},{21,0.},{12,1.},{22,0.},{13,0.},{23,1.},
		{74,int16_t{2}},{41,1.},{41,0.},{75,int16_t{0}}, {74,int16_t{2}},{41,-1.},{41,0.},{75,int16_t{0}},
		{11,10.},{21,0.},{12,1.},{22,0.},{13,0.},{23,1.},
		{74,int16_t{2}},{41,1.},{41,0.},{75,int16_t{0}}, {74,int16_t{2}},{41,-1.},{41,0.},{75,int16_t{0}}};
	geometry=line.GetRenderGeometry();REQUIRE(geometry.lines.size()==2);
	CHECK((geometry.lines[0].pt0==point_t{0.,1.,0.}));CHECK((geometry.lines[0].pt1==point_t{10.,1.,0.}));
	CHECK((geometry.lines[1].pt0==point_t{0.,-1.,0.}));
	for(auto& tag:helix.m_groups)if(tag.code==41)tag.value=1e100;
	CHECK(helix.GetRenderGeometry().Empty());
}

TEST_CASE("CAD renderer table cells and ROI", "[cad][render]") {
	using namespace gtl::shape;
	xTable table;table.m_groups={{100,L"AcDbEntity"s},{92,int32_t{300}}, {100,L"AcDbBlockReference"s},{10,2.},{20,20.},
		{100,L"AcDbTable"s},{91,int32_t{2}},{92,int32_t{2}},{141,5.},{141,5.},{142,10.},{142,10.},
		{171,int16_t{1}},{1,L"A"s},{171,int16_t{1}},{1,L"B"s},{171,int16_t{1}},{1,L"C"s},{171,int16_t{1}},{1,L"D"s}};
	auto geometry=table.GetRenderGeometry();CHECK(geometry.lines.size()==6);REQUIRE(geometry.texts.size()==4);
	CHECK(geometry.texts[3].text==L"D");
	cad_canvas canvas;rect_t roi;roi.pt0()={0.,0.,0.};roi.pt1()={30.,30.,0.};CHECK(table.DrawROI(canvas,roi));
	CHECK(canvas.lines.size()==6);CHECK(canvas.texts.size()==4);
	roi.pt0()={100.,100.,0.};roi.pt1()={110.,110.,0.};CHECK_FALSE(table.DrawROI(canvas,roi));
	table.m_groups[7].value=int32_t{1000000};CHECK(table.GetRenderGeometry().Empty());
	xRay ray;ray.m_origin={-5.,2.,0.};gtl::xRect2d clip;clip.pt0()={0.,0.};clip.pt1()={10.,10.};canvas.clip=clip;
	canvas.lines.clear();ray.Draw(canvas);REQUIRE(canvas.lines.size()==1);CHECK((canvas.lines[0].pt0==point_t{0.,2.,0.}));
	xXLine xline;xline.m_origin={20.,3.,0.};canvas.lines.clear();xline.Draw(canvas);REQUIRE(canvas.lines.size()==1);
	CHECK((canvas.lines[0].pt1==point_t{10.,3.,0.}));
}

TEST_CASE("DXF dimensions and tables prefer cached display blocks", "[cad][render]") {
    auto path=std::filesystem::temp_directory_path()/"gtl.cad.blocks.dxf";sRemoveFile cleanup{path};
    {std::ofstream out(path);out << "0\nSECTION\n2\nBLOCKS\n0\nBLOCK\n100\nAcDbEntity\n8\n0\n100\nAcDbBlockBegin\n2\nDISPLAY\n70\n0\n10\n1\n20\n2\n30\n0\n0\nLINE\n100\nAcDbEntity\n8\n0\n100\nAcDbLine\n10\n1\n20\n2\n30\n0\n11\n5\n21\n2\n31\n0\n0\nENDBLK\n100\nAcDbEntity\n8\n0\n100\nAcDbBlockEnd\n0\nENDSEC\n0\nSECTION\n2\nENTITIES\n0\nDIMENSION\n100\nAcDbEntity\n8\n0\n100\nAcDbDimension\n2\nDISPLAY\n70\n0\n12\n20\n22\n30\n0\nACAD_TABLE\n100\nAcDbEntity\n8\n0\n100\nAcDbBlockReference\n2\nDISPLAY\n10\n100\n20\n200\n11\n0\n21\n1\n0\nENDSEC\n0\nEOF\n";}
    auto drawing=gtl::dxf::ReadDXFShape(path);REQUIRE(drawing);auto layer=FindLayer(*drawing,L"0");REQUIRE(layer);REQUIRE(layer->m_shapes.size()==4);
    auto line=dynamic_cast<gtl::shape::xLine const*>(&layer->m_shapes[0]);REQUIRE(line);CHECK((line->m_pt0==gtl::shape::point_t{21,32,0}));
    auto tableLine=dynamic_cast<gtl::shape::xLine const*>(&layer->m_shapes[2]);REQUIRE(tableLine);CHECK(std::abs(tableLine->m_pt0.x-100)<1e-9);CHECK(std::abs(tableLine->m_pt0.y-200)<1e-9);
    for(size_t index:{1u,3u}){auto cad=dynamic_cast<gtl::shape::xCadEntity const*>(&layer->m_shapes[index]);REQUIRE(cad);CHECK(cad->m_bExternalGraphics);CHECK(cad->GetRenderGeometry().Empty());}
    std::stringstream buffer;{boost::archive::binary_oarchive archive(buffer);archive & *drawing;}
    gtl::shape::xDrawing restored;{boost::archive::binary_iarchive archive(buffer);archive & restored;}
    CHECK(dynamic_cast<gtl::shape::xCadEntity const&>(restored.m_layers.front().m_shapes[1]).m_bExternalGraphics);
}
TEST_CASE("Mat canvas clipping bounds enclose all inverse transformed corners", "[cad][render]") {
    cv::Mat image=cv::Mat::zeros(20,40,CV_8UC3);
    gtl::xCoordTrans3d ct;ct.m_mat=ct.GetRotatingMatrixXY(gtl::deg_t{45});gtl::shape::xCanvasMat canvas(image,ct);
    auto bounds=canvas.GetClippingRect();REQUIRE(bounds);
    for(auto p:{gtl::xPoint2d{0,0},{40,0},{0,20},{40,20}}){auto q=canvas.m_ctI(p);CHECK(q.x>=bounds->pt0().x);CHECK(q.x<=bounds->pt1().x);CHECK(q.y>=bounds->pt0().y);CHECK(q.y<=bounds->pt1().y);}
}

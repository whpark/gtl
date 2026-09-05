#include "pch.h"
#include "gtl/dwg/dwg.h"
#include "gtl/shape/color_table.h"
#include "../dwg/detail/bit_stream.h"
#include "../dwg/detail/container.h"
#include "fixture.h"

#include <catch2/catch_approx.hpp>
#include <cstdlib>
#include <fstream>
#include <random>
#include <set>

namespace {
	struct xTemporaryFile {
		std::filesystem::path path = std::filesystem::temp_directory_path() /
			(std::wstring{L"gtl.dwg.\ud55c\uae00."} + std::to_wstring(std::random_device{}()) + L".dwg");
		~xTemporaryFile() { std::error_code ec; std::filesystem::remove(path, ec); }
		void Write(std::vector<std::uint8_t> const& bytes) const {
			std::ofstream file(path, std::ios::binary);
			file.write(reinterpret_cast<char const*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
			REQUIRE(file.good());
		}
	};
	std::filesystem::path SampleFolder() {
		if (auto env = std::getenv("GTL_DWG_TEST_DIR")) return std::filesystem::path{env};
#ifdef GTL_DWG_SAMPLE_DIR
		return GTL_DWG_SAMPLE_DIR;
#else
		return std::filesystem::path{__FILE__}.parent_path() / "DWG";
#endif
	}
}

TEST_CASE("dwg bit streams decode known encodings and reject truncation", "[dwg][unit]") {
	using namespace gtl::dwg::detail;
	SECTION("unaligned little endian and bitshort") {
		std::uint8_t bytes[]{0x00, 0x40, 0x6d, 0x0f, 0x80};
		xBitStream stream{bytes};
		CHECK(stream.BS() == 257);
		CHECK(stream.BS() == 0);
		CHECK(stream.BS() == 256);
		CHECK(stream.BS() == 15);
		CHECK(stream.BS() == 0);
	}
	SECTION("signed modular char uses sign magnitude") {
		std::uint8_t bytes[]{0x85, 0x4b};
		xBitStream stream{bytes};
		CHECK(stream.MC() == -1413);
	}
	SECTION("relative handles") {
		std::uint8_t bytes[]{0xa1, 0x05, 0xc1, 0x03, 0x60, 0x80};
		xBitStream stream{bytes};
		CHECK(stream.Handle(10) == 15);
		CHECK(stream.Handle(10) == 7);
		CHECK(stream.Handle(10) == 11);
		CHECK(stream.Handle(10) == 9);
	}
	SECTION("default doubles preserve high bytes") {
		fixture::Bits bits;
		bits.Put(1, 2); bits.Raw(0x12345678, 4);
		xBitStream stream{bits.data};
		CHECK(std::bit_cast<std::uint64_t>(stream.DD(1.)) == 0x3ff0000012345678ULL);
	}
	SECTION("six byte default doubles use the specified byte order") {
		std::uint8_t bytes[]{0x80, 0x48, 0x8c, 0xd1, 0x15, 0x59, 0x80};
		xBitStream stream{bytes};
		CHECK(std::bit_cast<std::uint64_t>(stream.DD(1.)) == 0x3ff0220166554433ULL);
	}
	SECTION("CRC known reference") {
		std::string_view text = "123456789";
		CHECK(CRC16({reinterpret_cast<std::uint8_t const*>(text.data()), text.size()}, 0) == 0xbb3d);
	}
	SECTION("invalid inputs") {
		std::uint8_t bytes[]{0xff};
		xBitStream stream{bytes};
		CHECK_THROWS_AS(stream.BL(), xParseError);
		CHECK_THROWS_AS(stream.RD(), xParseError);
		CHECK_THROWS_AS(stream.Seek(9), xParseError);
		xBitStream unterminated{bytes};
		CHECK_THROWS_AS(unterminated.UMC(), xParseError);
	}
}

TEST_CASE("dwg truncated files and unsupported versions are explicit failures", "[dwg][unit]") {
	xTemporaryFile file;
	auto bytes = fixture::Drawing();
	SECTION("every truncated prefix") {
		gtl::dwg::xDWG dwg;
		for (size_t length{}; length < bytes.size(); ++length) {
			CAPTURE(length);
			file.Write({bytes.begin(), bytes.begin() + length});
			CHECK_FALSE(dwg.ReadDWG(file.path));
			CHECK_FALSE(dwg.IsLoaded());
			CHECK(dwg.GetDocument().objects.empty());
		}
	}
	SECTION("known unsupported and unknown signatures") {
		for (std::string_view signature : {"AC1018", "AC1021", "AC1024", "AC1027", "AC1032", "NOTDWG"}) {
			std::copy(signature.begin(), signature.end(), bytes.begin());
			file.Write(bytes);
			gtl::dwg::sReadReport report;
			CHECK_FALSE(gtl::dwg::ReadDWGShape(file.path, &report));
			CHECK(report.error == gtl::dwg::eREAD_ERROR::unsupported_version);
		}
	}
	SECTION("missing file") {
		gtl::dwg::xDWG dwg;
		CHECK_FALSE(dwg.ReadDWG(file.path));
		CHECK(dwg.GetReport().error == gtl::dwg::eREAD_ERROR::io);
	}
}

TEST_CASE("dwg R2000 fixture converts coordinates colors and Korean layer names", "[dwg][unit]") {
	xTemporaryFile file;
	file.Write(fixture::Drawing());
	gtl::dwg::xDWG dwg;
	REQUIRE(dwg.ReadDWG(file.path));
	REQUIRE(dwg.GetDocument().entities.size() == 2);
	gtl::dwg::sReadReport report;
	auto drawing = gtl::dwg::ToShape(dwg, &report);
	CHECK(report.diagnostics.empty());
	CHECK(report.convertedEntities == 2);
	REQUIRE(drawing.m_layers.size() == 1);
	auto const& layer = drawing.m_layers.front();
	CHECK(layer.m_name == L"\uac00");
	REQUIRE(layer.m_shapes.size() == 2);
	auto const* line = dynamic_cast<gtl::shape::xLine const*>(&layer.m_shapes[0]);
	REQUIRE(line);
	CHECK((line->m_pt0 == gtl::shape::point_t{1., 2., 3.}));
	CHECK((line->m_pt1 == gtl::shape::point_t{4., 5., 6.}));
	CHECK(line->m_color == gtl::shape::colorTable_s[1]);
	auto const* circle = dynamic_cast<gtl::shape::xCircle const*>(&layer.m_shapes[1]);
	REQUIRE(circle);
	CHECK((circle->m_ptCenter == gtl::shape::point_t{10., 20., 0.}));
	CHECK(circle->m_radius == 5.);
	CHECK(circle->m_color == gtl::shape::colorTable_s[3]);
	CHECK(drawing.m_rectBoundary.left == 1.);
	CHECK(drawing.m_rectBoundary.top == 2.);
	CHECK(drawing.m_rectBoundary.right == 15.);
	CHECK(drawing.m_rectBoundary.bottom == 25.);
	CHECK(gtl::dwg::ReadDWGShape(file.path).has_value());
}

TEST_CASE("dwg failures clear previously loaded state", "[dwg][unit]") {
	xTemporaryFile file;
	auto bytes = fixture::Drawing();
	file.Write(bytes);
	gtl::dwg::xDWG dwg;
	REQUIRE(dwg.ReadDWG(file.path));
	SECTION("bad object CRC") { bytes[100] ^= 1; }
	SECTION("bad header CRC") { bytes[19] ^= 1; }
	SECTION("bad map CRC") { bytes.back() ^= 1; }
	SECTION("truncated") { bytes.resize(69); }
	file.Write(bytes);
	CHECK_FALSE(dwg.ReadDWG(file.path));
	CHECK(dwg.GetReport().error == gtl::dwg::eREAD_ERROR::invalid_data);
	CHECK_FALSE(dwg.IsLoaded());
	CHECK(dwg.GetDocument().entities.empty());
	CHECK_THROWS_AS(gtl::dwg::ToShape(dwg), std::logic_error);
	gtl::dwg::sReadReport report;
	CHECK_FALSE(gtl::dwg::ReadDWGShape(file.path, &report));
	CHECK(report.error == gtl::dwg::eREAD_ERROR::invalid_data);
}

TEST_CASE("dwg legacy polyline follows handles and preserves elevation and bulges", "[dwg][unit]") {
	for (bool r2000 : {false,true}) for (bool three : {false,true}) {
		CAPTURE(r2000,three);
		fixture::Graph graph{r2000}; graph.Layer(1,"0"); graph.Polyline(three);
		xTemporaryFile file; file.Write(graph.Bytes());
		gtl::dwg::xDWG dwg; REQUIRE(dwg.ReadDWG(file.path));
		auto const& poly = std::get<gtl::dwg::entities::sPolyline>(dwg.GetDocument().entities.front().geometry);
		REQUIRE(poly.points.size()==2);
		CHECK(poly.closed); CHECK(poly.is3d==three);
		CHECK(poly.points[0].x==4.); CHECK(poly.points[1].x==7.);
		CHECK(poly.points[0].z==(three?6.:9.));
		CHECK(poly.bulges[0]==(three?0.:.5));
		if(!three) { CHECK(poly.widths[0].first==2.); CHECK(poly.widths[0].second==2.); }
		gtl::dwg::sReadReport report; auto drawing=gtl::dwg::ToShape(dwg,&report);
		CHECK(report.convertedEntities==1);
		auto const* shape=dynamic_cast<gtl::shape::xPolyline const*>(&drawing.m_layers.front().m_shapes.front());
		REQUIRE(shape); CHECK(shape->m_pts.size()==2); CHECK(shape->m_bLoop);
	}
}

TEST_CASE("dwg invalid entity graphs fail explicitly", "[dwg][unit]") {
	fixture::Graph graph; graph.Layer(1,"0");
	SECTION("wrong vertex owner") {graph.Polyline(false,99);}
	SECTION("vertex link cycle") {graph.Polyline(false,2,4);}
	SECTION("missing vertex") {graph.Polyline(false,2,6);}
	SECTION("missing block") {graph.Insert(2,0,99);}
	SECTION("wrong block member owner") {graph.Block(10,20,20);graph.Circle(20,99);}
	SECTION("missing block marker") {graph.Block(10,20,20);graph.Circle(20,10);graph.objects.erase(11);}
	SECTION("wrong block marker type") {graph.Block(10,20,20);graph.Circle(20,10);graph.Circle(11,10);}
	xTemporaryFile file; file.Write(graph.Bytes());
	gtl::dwg::xDWG dwg; CHECK_FALSE(dwg.ReadDWG(file.path));
	CHECK(dwg.GetReport().error==gtl::dwg::eREAD_ERROR::invalid_data);
	CHECK_FALSE(dwg.IsLoaded()); CHECK(dwg.GetDocument().entities.empty());
}

TEST_CASE("dwg nested block arrays apply bases scales rotations and inherited styles", "[dwg][unit]") {
	for (bool r2000 : {false,true}) {
		CAPTURE(r2000);
		fixture::Graph graph{r2000}; graph.Layer(1,std::string_view{"0\0",2}); graph.Layer(2,"visible",5);
		graph.Block(10,20,20,{1,2,0}); graph.Circle(20,10);
		graph.Block(30,40,40); graph.Insert(40,30,10,{3,4,0},{2,1,1},0.,1,1,1,{0,0,1},0);
		graph.Insert(50,0,30,{100,200,0},{1,1,1},std::numbers::pi/2,2,2,2);
		xTemporaryFile file; file.Write(graph.Bytes());
		gtl::dwg::xDWG dwg; REQUIRE(dwg.ReadDWG(file.path));
		CHECK(dwg.GetDocument().layers.at(1).name=="0");
		CHECK(dwg.GetDocument().blocks.at(10).entities==std::vector<gtl::dwg::handle_t>{20});
		gtl::dwg::sReadReport report; auto drawing=gtl::dwg::ToShape(dwg,&report);
		CHECK(report.diagnostics.empty()); REQUIRE(report.convertedEntities==4);
		REQUIRE(drawing.m_layers.size()==2); CHECK(drawing.m_layers[0].m_shapes.empty());
		auto const& shapes=drawing.m_layers[1].m_shapes; REQUIRE(shapes.size()==4);
		for(size_t i=0;i<4;++i) {
			auto const* ellipse=dynamic_cast<gtl::shape::xEllipse const*>(&shapes[i]); REQUIRE(ellipse);
			CHECK(ellipse->m_ptCenter.x==Catch::Approx(96.-20.*(i/2)));
			CHECK(ellipse->m_ptCenter.y==Catch::Approx(203.+10.*(i%2)));
			CHECK(ellipse->m_radius==Catch::Approx(4.)); CHECK(ellipse->m_radiusH==Catch::Approx(2.));
			CHECK(ellipse->m_color==gtl::shape::colorTable_s[1]); CHECK(ellipse->m_strLayer==L"visible");
		}
		CHECK(drawing.m_rectBoundary.left==Catch::Approx(74.)); CHECK(drawing.m_rectBoundary.right==Catch::Approx(98.));
		CHECK(drawing.m_rectBoundary.top==Catch::Approx(199.)); CHECK(drawing.m_rectBoundary.bottom==Catch::Approx(217.));
	}
}

TEST_CASE("dwg conversion bounds cycles arrays and unsupported transforms", "[dwg][unit]") {
	fixture::Graph graph; graph.Layer(1,"0"); graph.Block(10,20,20); graph.Circle(20,10);
	gtl::dwg::sShapeOptions options;
	std::string expected;
	SECTION("cycle") {graph.Insert(20,10,10);graph.Insert(30,0,10);expected="cyclic";}
	SECTION("depth") {graph.Insert(30,0,10);options.maxBlockDepth=0;expected="nesting limit";}
	SECTION("entity budget") {graph.Insert(30,0,10,{}, {1,1,1},0,2,2);options.maxEntities=1;expected="entity limit";}
	SECTION("empty array visit budget") {graph.Block(10,0,0);graph.objects.erase(20);graph.Insert(30,0,10,{}, {1,1,1},0,32767,32767);options.maxVisits=10;expected="visit limit";}
	SECTION("tilted circle") {graph.Insert(30,0,10,{}, {1,1,1},0,1,1,1,{0,1,0});expected="tilted";}
	SECTION("singular circle") {graph.Insert(30,0,10,{}, {0,1,1});expected="singular";}
	xTemporaryFile file;file.Write(graph.Bytes());gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));
	gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report,options);
	CHECK(report.convertedEntities<=1);
	CHECK(std::ranges::any_of(report.diagnostics,[&](auto const& d){return d.message.find(expected)!=std::string::npos;}));
}

TEST_CASE("rotated ellipse arc boundary includes only swept extrema", "[dwg][unit]") {
	gtl::shape::xEllipse ellipse;
	ellipse.m_radius=4.;ellipse.m_radiusH=2.;ellipse.m_angle_first_axis=gtl::deg_t{90.};
	ellipse.m_angle_start=gtl::deg_t{0.};ellipse.m_angle_length=gtl::deg_t{-90.};
	gtl::shape::rect_t bounds;bounds.SetRectEmptyForMinMax();ellipse.UpdateBoundary(bounds);
	CHECK(bounds.left==Catch::Approx(0.).margin(1e-12));CHECK(bounds.right==Catch::Approx(2.));
	CHECK(bounds.top==Catch::Approx(0.).margin(1e-12));CHECK(bounds.bottom==Catch::Approx(4.));
}

TEST_CASE("dwg reflected and sheared arcs retain their parameterized geometry", "[dwg][unit]") {
	for (bool nonuniform : {false,true}) {
		fixture::Graph graph;graph.Layer(1,"0");graph.Block(10,20,20);
		graph.Entity(0x11,20,10,[](fixture::Bits& b) {
			b.Point({1,2,0});b.Double(2.);b.Put(3,2);b.Double(.3);b.Double(2.1);
		});
		graph.Block(30,40,40);graph.Insert(40,30,10,{3,4,0},{2,nonuniform?1.:2.,1},.7);
		graph.Insert(50,0,30,{100,200,0},{-1,nonuniform?3.:1.,1},-.4);
		xTemporaryFile file;file.Write(graph.Bytes());gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));
		gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);
		REQUIRE(report.convertedEntities==1);CHECK(report.diagnostics.empty());
		auto const* arc=dynamic_cast<gtl::shape::xArc const*>(&drawing.m_layers.front().m_shapes.front());REQUIRE(arc);
		auto const* ellipse=dynamic_cast<gtl::shape::xEllipse const*>(arc);CHECK(bool(ellipse)==nonuniform);
		CHECK((double)arc->m_angle_length<0.);
		for(int i=0;i<=20;++i) {
			double fraction=i/20.;double t=.3+1.8*fraction;
			double x=2.*(1.+2.*std::cos(t)),y=(nonuniform?1.:2.)*(2.+2.*std::sin(t));
			double u=3.+std::cos(.7)*x-std::sin(.7)*y,v=4.+std::sin(.7)*x+std::cos(.7)*y;
			u=-u;v*=nonuniform?3.:1.;
			double expectedX=100.+std::cos(-.4)*u-std::sin(-.4)*v,expectedY=200.+std::sin(-.4)*u+std::cos(-.4)*v;
			double parameter=(double)(gtl::rad_t)arc->m_angle_start+(double)(gtl::rad_t)arc->m_angle_length*fraction;
			double a=arc->m_radius*std::cos(parameter),b=(ellipse?ellipse->m_radiusH:arc->m_radius)*std::sin(parameter);
			double angle=ellipse?(double)(gtl::rad_t)ellipse->m_angle_first_axis:0.;
			double actualX=arc->m_ptCenter.x+std::cos(angle)*a-std::sin(angle)*b, actualY=arc->m_ptCenter.y+std::sin(angle)*a+std::cos(angle)*b;
			CHECK(actualX==Catch::Approx(expectedX));CHECK(actualY==Catch::Approx(expectedY));
			CHECK(actualX>=drawing.m_rectBoundary.left-1e-9);CHECK(actualX<=drawing.m_rectBoundary.right+1e-9);
			CHECK(actualY>=drawing.m_rectBoundary.top-1e-9);CHECK(actualY<=drawing.m_rectBoundary.bottom+1e-9);
		}
	}
}

TEST_CASE("dwg corpus checks all supplied versions without modifying files", "[dwg][corpus]") {
	auto folder = SampleFolder();
	if (!std::filesystem::is_directory(folder)) {
		if (std::getenv("GTL_DWG_TEST_DIR")) FAIL("GTL_DWG_TEST_DIR does not exist");
		SKIP("DWG corpus not present; set GTL_DWG_TEST_DIR");
	}
	size_t files{};
	for (auto const& entry : std::filesystem::directory_iterator(folder)) {
		if (!entry.is_regular_file()) continue;
		auto extension = entry.path().extension().wstring();
		if (extension != L".dwg" && extension != L".DWG") continue;
		++files;
		CAPTURE(entry.path().filename().string());
		auto beforeSize = entry.file_size();
		auto beforeTime = entry.last_write_time();
		gtl::dwg::xDWG dwg;
		bool ok = dwg.ReadDWG(entry.path());
		auto const& report = dwg.GetReport();
		CAPTURE(report.message);
		if (report.version == gtl::dwg::eVERSION::r14 || report.version == gtl::dwg::eVERSION::r2000) {
			REQUIRE(ok);
			CHECK_FALSE(dwg.GetDocument().objects.empty());
			gtl::dwg::sReadReport converted;
			auto drawing = gtl::dwg::ToShape(dwg, &converted);
			CHECK(converted.convertedEntities > 0);
			CHECK_FALSE(drawing.m_layers.empty());
		}
		else {
			CHECK_FALSE(ok);
			CHECK(report.error == gtl::dwg::eREAD_ERROR::unsupported_version);
			CHECK(dwg.GetDocument().objects.empty());
			if(report.version>=gtl::dwg::eVERSION::r2004) {
				gtl::dwg::sReadReport containerReport;
				auto container=gtl::dwg::ReadDWGContainer(entry.path(),&containerReport);
				INFO(containerReport.message);REQUIRE(container);
				CHECK(containerReport.error==gtl::dwg::eREAD_ERROR::none);
				CHECK(container->version==report.version);
				for(auto name:{"AcDb:Header","AcDb:Classes","AcDb:Handles","AcDb:AcDbObjects"}) {
					REQUIRE(container->sections.contains(name));CHECK_FALSE(container->sections.at(name).data.empty());
				}
			}
		}
		CHECK(std::filesystem::file_size(entry.path()) == beforeSize);
		CHECK(std::filesystem::last_write_time(entry.path()) == beforeTime);
	}
	CHECK(files > 0);
}

TEST_CASE("dwg compression decodes independent literal and overlapping match vectors", "[dwg][unit]") {
	using namespace gtl::dwg::detail;
	SECTION("R2004 literals and overlaps") {
		std::vector<std::uint8_t> input{1,'a','b','c','d',0x40,0,0x11};
		CHECK(Decompress2004(input,7)==std::vector<std::uint8_t>{'a','b','c','d','d','d','d'});
		for(size_t n:{4u,18u,19u,273u,274u,1024u}) {
			std::vector<std::uint8_t> raw(n);for(size_t i=0;i<n;++i)raw[i]=static_cast<std::uint8_t>(i);
			CHECK(Decompress2004(fixture::Literal2004(raw),n)==raw);
		}
		CHECK_THROWS_AS(Decompress2004(input,6),xParseError);
		CHECK_THROWS_AS(Decompress2004(input,8),xParseError);
		input[6]=255;CHECK_THROWS_AS(Decompress2004(input,7),xParseError);
	}
	SECTION("R2007 literal byte order") {
		std::map<unsigned,std::vector<std::uint8_t>> known{
			{1,{0}}, {2,{1,0}}, {3,{2,1,0}}, {7,{6,5,1,2,3,4,0}},
			{16,{8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7}},
			{19,{18,17,16,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7}},
			{32,{24,25,26,27,28,29,30,31,16,17,18,19,20,21,22,23,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7}}
		};
		for(auto const& [n,expected]:known) {
			CAPTURE(n);std::vector<std::uint8_t> encoded;
			if(n<8)encoded={0x20,0,0,static_cast<std::uint8_t>(n)};
			else if(n<23)encoded={static_cast<std::uint8_t>(n-8)};
			else encoded={0x0f,static_cast<std::uint8_t>(n-23)};
			for(unsigned i=0;i<n;++i)encoded.push_back(static_cast<std::uint8_t>(i));
			CHECK(Decompress2007(encoded,n)==expected);
			for(size_t i=0;i<encoded.size();++i)CHECK_THROWS_AS(Decompress2007(std::span{encoded}.first(i),n),xParseError);
		}
		std::vector<std::uint8_t> encoded{0,'a','b','c','d','e','f','g','h',0x30,0};
		CHECK(Decompress2007(encoded,11)==std::vector<std::uint8_t>{'a','b','c','d','e','f','g','h','h','h','h'});
		encoded.back()=0xf8;CHECK_THROWS_AS(Decompress2007(encoded,11),xParseError);
	}
}

TEST_CASE("dwg compressed container API validates checksums and resource limits", "[dwg][unit]") {
	xTemporaryFile file;auto bytes=fixture::CompressedContainer();
	SECTION("four signatures share the R2004 container") {
		for(std::string_view signature:{"AC1018","AC1024","AC1027","AC1032"}) {
			file.Write(fixture::CompressedContainer(signature));gtl::dwg::sReadReport report;
			auto result=gtl::dwg::ReadDWGContainer(file.path,&report);INFO(report.message);REQUIRE(result);
			CHECK(report.error==gtl::dwg::eREAD_ERROR::none);REQUIRE(result->sections.size()==1);
			auto const& data=result->sections.at("AcDb:Test").data;
			CHECK(std::string(data.begin(),data.end())=="abcdefghijkl");
		}
	}
	SECTION("corrupt file header, data header, data payload, system header and map") {
		for(size_t at:{128u,256u,288u,352u,380u,608u,630u}) {
			auto corrupt=bytes;corrupt[at]^=1;file.Write(corrupt);gtl::dwg::sReadReport report;
			CHECK_FALSE(gtl::dwg::ReadDWGContainer(file.path,&report));CHECK(report.error==gtl::dwg::eREAD_ERROR::invalid_data);
		}
	}
	SECTION("limits") {
		file.Write(bytes);
		for(auto options:{gtl::dwg::sContainerOptions{100,10000,10},gtl::dwg::sContainerOptions{10000,10,10},gtl::dwg::sContainerOptions{10000,10000,1}}) {
			gtl::dwg::sReadReport report;CHECK_FALSE(gtl::dwg::ReadDWGContainer(file.path,&report,options));CHECK(report.error==gtl::dwg::eREAD_ERROR::resource_limit);
		}
	}
	SECTION("truncated system map") {
		for(size_t size:{0u,5u,128u,235u,607u,627u,640u}) {
			file.Write({bytes.begin(),bytes.begin()+size});CHECK_FALSE(gtl::dwg::ReadDWGContainer(file.path));
		}
	}
}

TEST_CASE("dwg R2007 corpus rejects damaged RS blocks and truncated pages", "[dwg][corpus]") {
	auto sample=SampleFolder()/"sample_AC1021.dwg";
	if(!std::filesystem::exists(sample))SKIP("R2007 reference corpus not present");
	std::ifstream input(sample,std::ios::binary);
	std::vector<std::uint8_t> original{std::istreambuf_iterator<char>{input},{}};
	xTemporaryFile file;
	// The supplied reference has header at 0x80, system map at 0x480,
	// and non-interleaved AppInfoHistory at 30176. Mutate temporary copies only.
	for(size_t at:{128u,128u+239u*3u,0x480u,30176u}) {
		REQUIRE(at<original.size());auto bytes=original;bytes[at]^=1;file.Write(bytes);
		gtl::dwg::sReadReport report;CHECK_FALSE(gtl::dwg::ReadDWGContainer(file.path,&report));
		CHECK(report.error==gtl::dwg::eREAD_ERROR::invalid_data);
	}
	for(size_t length:{764u,0x480u,32000u}) {
		file.Write({original.begin(),original.begin()+length});CHECK_FALSE(gtl::dwg::ReadDWGContainer(file.path));
	}
}

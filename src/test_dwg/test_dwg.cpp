#include "pch.h"
#include "gtl/dwg/dwg.h"
#include "gtl/shape/color_table.h"
#include "../dwg/detail/bit_stream.h"
#include "../dwg/detail/container.h"
#include "../dwg/detail/reader.h"
#include "../dwg/detail/curves.h"
#include "../dwg/detail/hatch.h"
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
		for (std::string_view signature : {"AC1009", "AC9999", "NOTDWG"}) {
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

TEST_CASE("dwg native ellipses retain WCS axes and reflected arc parameters", "[dwg][unit]") {
	for(bool modern:{false,true})for(bool reflected:{false,true})for(bool full:{false,true}) {
		CAPTURE(modern,reflected,full);
		fixture::Graph graph;graph.modern=modern;graph.Layer(1,"0");graph.Block(10,20,20);
		graph.Entity(0x23,20,10,[&](fixture::Bits& b) {
			b.Point({10,20,3});b.Point({3,4,0});b.Point({0,0,-2});b.Double(.4);
			b.Double(full?0.:5.5);b.Double(full?2*std::numbers::pi:.5);
		});
		graph.Insert(30,0,10,{100,200,0},{reflected?-2.:2.,3.,1},.3);
		xTemporaryFile file;file.Write(modern?fixture::PackSections(graph.Sections()):graph.Bytes());
		gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));
		gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);
		REQUIRE(report.convertedEntities==1);CHECK(report.diagnostics.empty());
		auto const* ellipse=dynamic_cast<gtl::shape::xEllipse const*>(&drawing.m_layers.front().m_shapes.front());REQUIRE(ellipse);
		double sweep=full?2*std::numbers::pi:2*std::numbers::pi-5.;
		CHECK((double)ellipse->m_angle_length==Catch::Approx((reflected?1.:-1.)*sweep*180/std::numbers::pi));
		for(int i=0;i<=20;++i) {
			double fraction=i/20.,t=(full?0.:5.5)+sweep*fraction;
			// WCS major=(3,4), minor=(1.6,-1.2); then INSERT scale and rotation.
			double x=(reflected?-2.:2.)*(10+3*std::cos(t)+1.6*std::sin(t));
			double y=3*(20+4*std::cos(t)-1.2*std::sin(t));
			double expectedX=100+std::cos(.3)*x-std::sin(.3)*y,expectedY=200+std::sin(.3)*x+std::cos(.3)*y;
			double parameter=(double)(gtl::rad_t)ellipse->m_angle_start+(double)(gtl::rad_t)ellipse->m_angle_length*fraction;
			double a=ellipse->m_radius*std::cos(parameter),b=ellipse->m_radiusH*std::sin(parameter);
			double angle=(double)(gtl::rad_t)ellipse->m_angle_first_axis;
			double actualX=ellipse->m_ptCenter.x+std::cos(angle)*a-std::sin(angle)*b;
			double actualY=ellipse->m_ptCenter.y+std::sin(angle)*a+std::cos(angle)*b;
			CHECK(actualX==Catch::Approx(expectedX));CHECK(actualY==Catch::Approx(expectedY));CHECK(ellipse->m_ptCenter.z==3.);
			CHECK(actualX>=drawing.m_rectBoundary.left-1e-9);CHECK(actualX<=drawing.m_rectBoundary.right+1e-9);
			CHECK(actualY>=drawing.m_rectBoundary.top-1e-9);CHECK(actualY<=drawing.m_rectBoundary.bottom+1e-9);
		}
	}
}

TEST_CASE("dwg ellipses decode across revisions and diagnose unsupported planes", "[dwg][unit]") {
	for(unsigned year:{14u,2000u,2004u,2007u,2010u,2013u,2018u}) {
		CAPTURE(year);
		auto geometry=[](fixture::Bits& b){b.Point({10,20,3});b.Point({3,4,0});b.Point({0,0,1});b.Double(.5);b.Double(0);b.Double(2*std::numbers::pi);};
		gtl::dwg::sDocument doc;
		if(year<=2004){fixture::Graph graph;graph.r2000=year!=14;graph.modern=year==2004;graph.Layer(1,"0");graph.Entity(0x23,2,0,geometry);
			xTemporaryFile file;file.Write(year==2004?fixture::PackSections(graph.Sections()):graph.Bytes());gtl::dwg::xDWG dwg;
			REQUIRE(dwg.ReadDWG(file.path));doc=dwg.GetDocument();
		}else {
			gtl::dwg::sContainer container;container.version=year==2007?gtl::dwg::eVERSION::r2007:year==2010?gtl::dwg::eVERSION::r2010:year==2013?gtl::dwg::eVERSION::r2013:gtl::dwg::eVERSION::r2018;
			for(auto const& [name,bytes]:fixture::UnicodeSections(year,u"test",false,0x23,geometry))container.sections[name].data=bytes;
			doc=gtl::dwg::detail::ReadContainerObjects(container);
		}
		REQUIRE(doc.entities.size()==1);auto const* ellipse=std::get_if<gtl::dwg::entities::sEllipse>(&doc.entities.front().geometry);
		REQUIRE(ellipse);CHECK(ellipse->ratio==.5);CHECK(ellipse->majorAxis.x==3.);CHECK(ellipse->center.z==3.);
	}
	for(int invalid:{0,1,2,3}) {
		fixture::Graph graph;graph.Layer(1,"0");graph.Entity(0x23,2,0,[&](fixture::Bits& b){
			b.Point({0,0,0});b.Point(invalid==2?std::array<double,3>{}:std::array<double,3>{3,0,0});
			b.Point(invalid==1?std::array<double,3>{0,1,0}:std::array<double,3>{0,0,1});
			b.Double(invalid==0?0.:invalid==3?2.:.5);b.Double(0);b.Double(1);
		});
		xTemporaryFile file;file.Write(graph.Bytes());gtl::dwg::xDWG dwg;
		if(invalid==0 || invalid==3){CHECK_FALSE(dwg.ReadDWG(file.path));continue;}
		REQUIRE(dwg.ReadDWG(file.path));gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);
		CHECK(report.convertedEntities==0);CHECK_FALSE(report.diagnostics.empty());
	}
}

TEST_CASE("dwg spline control and fit data survive all object stream revisions", "[dwg][unit]") {
	for(unsigned year:{14u,2000u,2004u,2007u,2010u,2013u,2018u})for(unsigned scenario:{1u,2u}) {
		CAPTURE(year,scenario);
		auto geometry=[&](fixture::Bits& b) {
			b.Long(scenario);if(year>=2013){b.Long(scenario==2?1:0);b.Long(scenario==2?0:15);}b.Long(2);
			if(scenario==1){b.Put(4,3);b.Double(1e-7);b.Double(1e-7);b.Long(6);b.Long(3);b.Put(1,1);
				for(double k:{2.,2.,2.,5.,5.,5.})b.Double(k);
				for(unsigned i=0;i<3;++i){b.Point({double(i),i==1?2.:0.,3.});b.Raw(std::bit_cast<std::uint64_t>(i==1?.5:1.),8);}
			}else {b.Double(1e-7);b.Point({1,0,0});b.Point({0,1,0});b.Long(3);for(unsigned i=0;i<3;++i)b.Point({double(i),double(i*i),0});}
		};
		gtl::dwg::sDocument doc;
		if(year<=2004){fixture::Graph graph;graph.r2000=year!=14;graph.modern=year==2004;graph.Layer(1,"0");graph.Entity(0x24,2,0,geometry);
			xTemporaryFile file;file.Write(year==2004?fixture::PackSections(graph.Sections()):graph.Bytes());gtl::dwg::xDWG dwg;
			REQUIRE(dwg.ReadDWG(file.path));doc=dwg.GetDocument();gtl::dwg::sReadReport report;gtl::dwg::ToShape(dwg,&report);
			CHECK(report.convertedEntities==1);CHECK_FALSE(report.diagnostics.empty());
		}else {gtl::dwg::sContainer container;container.version=year==2007?gtl::dwg::eVERSION::r2007:year==2010?gtl::dwg::eVERSION::r2010:year==2013?gtl::dwg::eVERSION::r2013:gtl::dwg::eVERSION::r2018;
			for(auto const& [name,bytes]:fixture::UnicodeSections(year,u"test",false,0x24,geometry))container.sections[name].data=bytes;
			doc=gtl::dwg::detail::ReadContainerObjects(container);
		}
		REQUIRE(doc.entities.size()==1);auto const* spline=std::get_if<gtl::dwg::entities::sSpline>(&doc.entities.front().geometry);REQUIRE(spline);
		CHECK(spline->degree==2);CHECK(spline->scenario==scenario);
		if(scenario==1){REQUIRE(spline->weights.size()==3);CHECK(spline->weights[1]==.5);CHECK(spline->knots.front()==2.);CHECK(spline->controlPoints[1].z==3.);}
		else {CHECK(spline->fitPoints.size()==3);CHECK(spline->startTangent.x==1.);CHECK(spline->endTangent.y==1.);}
	}
}

TEST_CASE("dwg spline conversion uses supplied knots and transformed control points", "[dwg][unit]") {
	fixture::Graph graph;graph.Layer(1,"0");graph.Block(10,20,20);
	graph.Entity(0x24,20,10,[](fixture::Bits& b){b.Long(1);b.Long(1);b.Put(0,3);b.Double(1e-7);b.Double(1e-7);b.Long(5);b.Long(3);b.Put(0,1);
		for(double k:{2.,2.,3.,6.,6.})b.Double(k);b.Point({0,0,0});b.Point({2,2,0});b.Point({4,0,0});});
	graph.Insert(30,0,10,{10,20,0},{-2,3,1});
	xTemporaryFile file;file.Write(graph.Bytes());gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));
	gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);REQUIRE(report.convertedEntities==1);CHECK(report.diagnostics.empty());
	auto const* spline=dynamic_cast<gtl::shape::xSpline const*>(&drawing.m_layers.front().m_shapes.front());REQUIRE(spline);
	CHECK((spline->m_ptsControl[1]==gtl::shape::point_t{6,26,0}));CHECK(spline->m_knots[2]==3.);
	struct Canvas:gtl::shape::ICanvas {
		std::vector<gtl::shape::point_t> points;
		void MoveTo_Target(gtl::shape::point_t const& p)override{points.push_back(p);}
		void LineTo_Target(gtl::shape::point_t const& p)override{points.push_back(p);}
		void PreDraw(gtl::shape::xShape const&)override{}
	} canvas;
	canvas.m_target_interpolation_inverval=2*std::hypot(4.,6.)/4; // sample fractions 0, .25, .5, .75, 1
	spline->Draw(canvas);REQUIRE(canvas.points.size()==5);
	CHECK(canvas.points[1].x==Catch::Approx(6.));CHECK(canvas.points[1].y==Catch::Approx(26.));
	CHECK(canvas.points.back().x==Catch::Approx(2.));CHECK(canvas.points.back().y==Catch::Approx(20.));
}

TEST_CASE("ellipse canvas draws negative sweeps through the final endpoint", "[dwg][unit]") {
	struct Canvas:gtl::shape::ICanvas {
		std::vector<gtl::shape::point_t> points;
		void MoveTo_Target(gtl::shape::point_t const& p)override{points.push_back(p);}
		void LineTo_Target(gtl::shape::point_t const& p)override{points.push_back(p);}
		void PreDraw(gtl::shape::xShape const&)override{}
	} canvas;
	canvas.Ellipse({10,20,0},4,2,gtl::deg_t{0},gtl::deg_t{0},gtl::deg_t{-90});
	REQUIRE(canvas.points.size()>2);CHECK(canvas.points.front().x==Catch::Approx(14.));
	CHECK(canvas.points.back().x==Catch::Approx(10.));CHECK(canvas.points.back().y==Catch::Approx(18.));
}

TEST_CASE("dwg malformed spline counts and knot domains fail explicitly", "[dwg][unit]") {
	for(int invalid:{0,1,2,3}) {
		fixture::Graph graph;graph.Layer(1,"0");graph.Entity(0x24,2,0,[&](fixture::Bits& b){
			b.Long(1);b.Long(invalid==0?0:1);b.Put(0,3);b.Double(1e-7);b.Double(1e-7);
			b.Long(invalid==1?0x7fffffff:5);b.Long(3);b.Put(0,1);
			for(double k:{2.,2.,invalid==2?1.:3.,invalid==3?2.:6.,invalid==3?2.:6.})b.Double(k);
			b.Point({0,0,0});b.Point({2,2,0});b.Point({4,0,0});
		});
		xTemporaryFile file;file.Write(graph.Bytes());gtl::dwg::xDWG dwg;CHECK_FALSE(dwg.ReadDWG(file.path));
		CHECK(dwg.GetReport().error==gtl::dwg::eREAD_ERROR::invalid_data);
	}
}

TEST_CASE("dwg text and attributes decode native strings across all revisions", "[dwg][unit]") {
	for(unsigned year:{14u,2000u,2004u,2007u,2010u,2013u,2018u})for(unsigned type:{1u,2u,3u}) {
		CAPTURE(year,type);
		auto geometry=[&](fixture::Bits& b){
			if(year!=14){b.Raw(255,1);b.Raw(std::bit_cast<std::uint64_t>(10.),8);b.Raw(std::bit_cast<std::uint64_t>(20.),8);b.Put(3,2);b.Raw(std::bit_cast<std::uint64_t>(2.),8);}
			else{b.Double(0);for(double v:{10.,20.,10.,20.})b.Raw(std::bit_cast<std::uint64_t>(v),8);b.Point({0,0,1});b.Double(0);b.Double(.1);b.Double(.2);b.Double(2);b.Double(1);}
			if(year<2007)b.Text("Hello");if(year==14){b.Short(0);b.Short(0);b.Short(0);}
			if(type!=1){if(year>=2010)b.Raw(0,1);if(year>=2018)b.Raw(1,1);if(year<2007)b.Text("TAG");b.Short(0);b.Raw(2,1);if(year>=2007)b.Put(1,1);
				if(type==3){if(year>=2010)b.Raw(0,1);if(year<2007)b.Text("Prompt");}}
		};
		auto strings=[&](fixture::Bits& b){auto text=[&](std::u16string_view value){b.Short(static_cast<int>(value.size()));for(auto c:value)b.Raw(c,2);};text(u"\uac00\U0001f642");if(type!=1)text(u"TAG");if(type==3)text(u"Prompt");};
		gtl::dwg::sDocument doc;
		if(year<=2004){fixture::Graph graph{year!=14};graph.modern=year==2004;graph.Layer(1,"0");graph.Entity(type,2,0,geometry,{0});xTemporaryFile file;file.Write(year==2004?fixture::PackSections(graph.Sections()):graph.Bytes());
			gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));doc=dwg.GetDocument();gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);REQUIRE(report.convertedEntities==1);
			auto const* text=dynamic_cast<gtl::shape::xText const*>(&drawing.m_layers.front().m_shapes.front());REQUIRE(text);CHECK(text->m_text==L"Hello");CHECK(text->m_height==Catch::Approx(2.));
		}else{gtl::dwg::sContainer container;container.version=year==2007?gtl::dwg::eVERSION::r2007:year==2010?gtl::dwg::eVERSION::r2010:year==2013?gtl::dwg::eVERSION::r2013:gtl::dwg::eVERSION::r2018;
			for(auto const& [name,bytes]:fixture::UnicodeSections(year,u"test",false,type,geometry,strings,{0}))container.sections[name].data=bytes;doc=gtl::dwg::detail::ReadContainerObjects(container);}
		REQUIRE(doc.entities.size()==1);auto const* text=std::get_if<gtl::dwg::entities::sText>(&doc.entities.front().geometry);REQUIRE(text);
		CHECK(text->insertion.x==10.);CHECK(text->insertion.y==20.);CHECK(text->height==2.);if(year>=2007)CHECK(text->text=="\xea\xb0\x80\xf0\x9f\x99\x82");
		if(type!=1){CHECK(text->tag=="TAG");CHECK(text->flags==2);CHECK(text->lockPosition==(year>=2007));}if(type==3)CHECK(text->prompt=="Prompt");
	}
}

TEST_CASE("dwg paper space selection and dimension blocks convert independently", "[dwg][unit]") {
	SECTION("paper selection") {
		fixture::Graph graph;graph.Layer(1,"0");graph.space=1;graph.Circle(2,0);xTemporaryFile file;file.Write(graph.Bytes());gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));
		gtl::dwg::sReadReport report;gtl::dwg::ToShape(dwg,&report);CHECK(report.convertedEntities==0);
		gtl::dwg::sShapeOptions options;options.space=gtl::dwg::eSPACE::paper;auto drawing=gtl::dwg::ToShape(dwg,&report,options);CHECK(report.convertedEntities==1);
		options.paperBlock=999;gtl::dwg::ToShape(dwg,&report,options);CHECK(report.convertedEntities==0);
	}
	SECTION("dimension anonymous block placement") {
		fixture::Graph graph;graph.Layer(1,"0");graph.Block(10,20,20);graph.Circle(20,10);
		graph.Entity(0x15,30,0,[](fixture::Bits& b){b.Point({0,0,1});b.Raw(0,8);b.Raw(0,8);b.Double(0);b.Raw(0,1);b.Text("42");b.Double(0);b.Double(0);b.Point({2,3,1});b.Double(0);
			b.Short(1);b.Short(1);b.Double(1);b.Double(42);b.Raw(std::bit_cast<std::uint64_t>(100.),8);b.Raw(std::bit_cast<std::uint64_t>(200.),8);for(int i=0;i<3;++i)b.Point({0,0,0});b.Double(0);b.Double(0);},{0,10});
		xTemporaryFile file;file.Write(graph.Bytes());gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);REQUIRE(report.convertedEntities==1);
		auto const* ellipse=dynamic_cast<gtl::shape::xEllipse const*>(&drawing.m_layers.front().m_shapes.front());REQUIRE(ellipse);CHECK(ellipse->m_ptCenter.x==102.);CHECK(ellipse->m_ptCenter.y==206.);
	}
}

TEST_CASE("dwg rational evaluation and fit interpolation preserve known geometry", "[dwg][unit]") {
	using namespace gtl::dwg;entities::sSpline curve;curve.degree=2;curve.scenario=1;curve.knots={0,0,0,1,1,1};curve.controlPoints={{1,0,0},{1,1,0},{0,1,0}};curve.weights={1,std::sqrt(.5),1};
	auto midpoint=detail::SplinePoint(curve,.5);CHECK(midpoint.x==Catch::Approx(std::sqrt(.5)));CHECK(midpoint.y==Catch::Approx(std::sqrt(.5)));
	auto points=detail::Tessellate(curve,{},.001,1000);REQUIRE(points.size()>10);for(auto p:points)CHECK(std::hypot(p.x,p.y)==Catch::Approx(1.));
	CHECK_THROWS(detail::Tessellate(curve,{},.001,2));CHECK_THROWS(detail::Tessellate(curve,{},0.,1000));
	entities::sSpline fit;fit.scenario=2;fit.degree=2;fit.fitPoints={{0,0,0},{1,2,3},{2,0,0}};auto interpolated=detail::Interpolate(fit);
	CHECK(detail::SplinePoint(interpolated,.5).y==Catch::Approx(2.));CHECK(detail::SplinePoint(interpolated,.5).z==Catch::Approx(3.));
	fit.degree=3;fit.startTangent={1,0,0};fit.endTangent={1,0,0};interpolated=detail::Interpolate(fit);
	CHECK(detail::SplinePoint(interpolated,.5).y==Catch::Approx(2.));CHECK(std::abs(detail::SplinePoint(interpolated,1e-5).y)<1e-6);
	fit.fitPoints[1]=fit.fitPoints[0];CHECK_THROWS(detail::Interpolate(fit));
}

TEST_CASE("dwg hatch patterns respect holes styles and bounded work", "[dwg][unit]") {
	using namespace gtl::dwg;entities::sHatch hatch;hatch.solid=true;
	for(auto [a,b]:{std::pair{0.,10.},std::pair{3.,7.}}){entities::sHatch::sPath path;path.flags=2;path.polyline.closed=true;path.polyline.points={{a,a,0},{b,a,0},{b,b,0},{a,b,0}};hatch.paths.push_back(path);}
	auto loops=detail::HatchLoops(hatch,.01,1000);REQUIRE(loops.size()==2);std::vector<entities::sLine> lines;
	detail::HatchPattern(hatch,loops,1.,1000,[&](auto a,auto b){lines.push_back({a,b});});
	CHECK(std::ranges::count_if(lines,[](auto const& line){return line.start.y==5.5;})==2);
	for(auto const& line:lines)if(line.start.y==5.5)CHECK((line.end.x<=3. || line.start.x>=7.));
	hatch.style=2;lines.clear();detail::HatchPattern(hatch,loops,1.,1000,[&](auto a,auto b){lines.push_back({a,b});});
	CHECK(std::ranges::any_of(lines,[](auto const& line){return line.start.y==5.5 && line.start.x<=3. && line.end.x>=7.;}));
	CHECK_THROWS(detail::HatchPattern(hatch,loops,.00001,10,[](auto,auto){}));
	std::vector<std::vector<point_t>> small{{{0,.1,0},{1,.1,0},{1,.2,0},{0,.2,0},{0,.1,0}}};size_t strokes{};
	detail::HatchPattern(hatch,small,1.,1000,[&](auto,auto){++strokes;});CHECK(strokes==1);
	hatch.solid=false;hatch.style=0;hatch.patternLines={{0,{},{0,2,0},{1,-1}}};lines.clear();detail::HatchPattern(hatch,loops,1.,1000,[&](auto a,auto b){lines.push_back({a,b});});
	for(auto const& line:lines)CHECK(line.end.x-line.start.x<=1.+1e-12);
}

TEST_CASE("dwg MTEXT and solid HATCH decode and convert independent fixtures", "[dwg][unit]") {
	for(unsigned year:{14u,2000u,2004u,2007u,2010u,2013u,2018u})for(unsigned type:{0x2cu,0x4eu}) {
		CAPTURE(year,type);bool hatch=type==0x4e;
		auto geometry=[&](fixture::Bits& b){
			if(!hatch){b.Point({10,20,3});b.Point({0,0,1});b.Point({1,0,0});b.Double(40);if(year>=2007)b.Double(12);b.Double(2);b.Short(1);b.Short(1);b.Double(12);b.Double(40);
				if(year<2007)b.Text("A\\PB");if(year>=2000){b.Short(1);b.Double(1);b.Put(0,1);}if(year>=2004)b.Long(0);if(year>=2018)b.Put(0,1);
			}else{if(year>=2004){b.Long(0);b.Long(0);b.Double(0);b.Double(0);b.Long(0);b.Double(0);b.Long(0);if(year<2007)b.Text("");}
				b.Double(0);b.Point({0,0,1});if(year<2007)b.Text("SOLID");b.Put(1,1);b.Put(0,1);b.Long(1);b.Long(3);b.Put(0,1);b.Put(1,1);b.Long(4);
				for(auto p:{std::pair{0.,0.},std::pair{10.,0.},std::pair{10.,10.},std::pair{0.,10.}}){b.Raw(std::bit_cast<std::uint64_t>(p.first),8);b.Raw(std::bit_cast<std::uint64_t>(p.second),8);}
				b.Long(0);b.Short(0);b.Short(1);b.Long(1);b.Raw(std::bit_cast<std::uint64_t>(1.),8);b.Raw(std::bit_cast<std::uint64_t>(1.),8);}
		};
		auto strings=[&](fixture::Bits& b){auto text=[&](std::u16string_view value){b.Short(static_cast<int>(value.size()));for(auto c:value)b.Raw(c,2);};if(hatch){text(u"");text(u"SOLID");}else text(u"A\\PB");};
		gtl::dwg::sDocument doc;
		if(year<=2004){fixture::Graph graph{year!=14};graph.modern=year==2004;graph.Layer(1,"0");graph.Entity(type,2,0,geometry,hatch?std::vector<unsigned>{}:std::vector<unsigned>{0});
			xTemporaryFile file;file.Write(year==2004?fixture::PackSections(graph.Sections()):graph.Bytes());gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));doc=dwg.GetDocument();
			gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);CHECK(report.convertedEntities==(hatch?10:1));
			if(hatch){gtl::dwg::sShapeOptions options;options.hatchBoundaryOnly=true;gtl::dwg::ToShape(dwg,&report,options);CHECK(report.convertedEntities==1);
				options.hatchBoundaryOnly=false;options.maxHatchSegments=2;gtl::dwg::ToShape(dwg,&report,options);CHECK(report.convertedEntities==0);CHECK_FALSE(report.diagnostics.empty());}
		}else{gtl::dwg::sContainer container;container.version=year==2007?gtl::dwg::eVERSION::r2007:year==2010?gtl::dwg::eVERSION::r2010:year==2013?gtl::dwg::eVERSION::r2013:gtl::dwg::eVERSION::r2018;
			for(auto const& [name,bytes]:fixture::UnicodeSections(year,u"test",false,type,geometry,strings,hatch?std::vector<unsigned>{}:std::vector<unsigned>{0}))container.sections[name].data=bytes;doc=gtl::dwg::detail::ReadContainerObjects(container);}
		REQUIRE(doc.entities.size()==1);
		if(hatch){auto const* value=std::get_if<gtl::dwg::entities::sHatch>(&doc.entities.front().geometry);REQUIRE(value);CHECK(value->name=="SOLID");REQUIRE(value->paths.size()==1);CHECK(value->paths.front().polyline.points.size()==4);CHECK(value->seeds.front().x==1.);}
		else{auto const* value=std::get_if<gtl::dwg::entities::sMText>(&doc.entities.front().geometry);REQUIRE(value);CHECK(value->text=="A\\PB");CHECK(value->width==40.);CHECK(value->textHeight==2.);}
	}
}

TEST_CASE("dwg LTYPE records produce bounded dash strokes", "[dwg][unit]") {
	fixture::Graph graph;graph.modern=true;graph.Layer(1,"0",3,{},5);
	fixture::Bits b;b.Short(0x39);auto end=b.position;b.Raw(0,4);b.Handle(5);b.Short(0);b.Long(0);b.Put(1,1);b.Text("DASHED");b.Put(0,1);b.Short(0);b.Put(0,1);b.Text("dash");b.Double(3);b.Raw(65,1);b.Raw(2,1);
	for(double dash:{2.,-1.}){b.Double(dash);b.Short(0);b.Raw(0,8);b.Raw(0,8);b.Double(1);b.Double(0);b.Short(0);}for(int i=0;i<256;++i)b.Raw(0,1);
	graph.Finish(5,b,end,[](fixture::Bits& b){for(int i=0;i<4;++i)b.Handle(0);});
	graph.Entity(0x13,2,0,[](fixture::Bits& b){b.Put(1,1);b.Raw(0,8);b.Put(3,2);b.Raw(std::bit_cast<std::uint64_t>(10.),8);b.Raw(0,8);b.Put(0,2);b.Put(3,2);});
	xTemporaryFile file;file.Write(fixture::PackSections(graph.Sections()));gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));CHECK(dwg.GetDocument().lineTypes.at(5).dashes.size()==2);
	gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);REQUIRE(report.convertedEntities==4);CHECK(report.diagnostics.empty());
	for(size_t i=0;i<4;++i){auto const* line=dynamic_cast<gtl::shape::xLine const*>(&drawing.m_layers.front().m_shapes[i]);REQUIRE(line);CHECK(line->m_pt0.x==Catch::Approx(3.*i));CHECK(line->m_pt1.x==Catch::Approx(std::min(10.,3.*i+2.)));}
	gtl::dwg::sShapeOptions options;options.maxEntities=2;gtl::dwg::ToShape(dwg,&report,options);CHECK(report.convertedEntities==2);CHECK_FALSE(report.diagnostics.empty());
}

TEST_CASE("dwg header metadata is checked independently of object CRCs", "[dwg][corpus]") {
	auto path=SampleFolder()/"sample_AC1032.dwg";if(!std::filesystem::exists(path))SKIP("reference sample unavailable");
	auto container=gtl::dwg::ReadDWGContainer(path);REQUIRE(container);auto doc=gtl::dwg::detail::ReadContainerObjects(*container);
	CHECK(doc.insertionUnits.has_value());CHECK(doc.measurement.has_value());CHECK(doc.headerVariables.contains("INSBASE_MSPACE"));CHECK(doc.headerVariables.contains("TDCREATE_DAY"));
	auto& header=container->sections.at("AcDb:Header").data;REQUIRE(header.size()>40);header[30]^=1;
	CHECK_THROWS_AS(gtl::dwg::detail::ReadContainerObjects(*container),gtl::dwg::detail::xParseError);
}

TEST_CASE("dwg template measurement accepts byte and Unicode-unit lengths", "[dwg][unit]") {
	for(std::uint8_t length:{1,2}){
		gtl::dwg::sContainer container;container.version=gtl::dwg::eVERSION::r2010;
		for(auto const& [name,bytes]:fixture::UnicodeSections(2010))container.sections[name].data=bytes;
		container.sections["AcDb:Template"].data={length,0,65,0,1,0};
		CHECK(gtl::dwg::detail::ReadContainerObjects(container).measurement==1);
		container.sections["AcDb:Template"].data[0]=10;CHECK_THROWS_AS(gtl::dwg::detail::ReadContainerObjects(container),gtl::dwg::detail::xParseError);
	}
}

TEST_CASE("dwg SOLID and TRACE retain corner order for dimension arrow fills", "[dwg][unit]") {
	for(bool r2000:{false,true})for(unsigned type:{0x1fu,0x20u}){
		fixture::Graph graph{r2000};graph.Layer(1,"0");graph.Entity(type,2,0,[&](fixture::Bits& b){if(r2000)b.Put(1,1);else b.Double(0);b.Double(3);
			for(auto p:{std::pair{0.,0.},std::pair{2.,0.},std::pair{0.,2.},std::pair{2.,2.}}){b.Raw(std::bit_cast<std::uint64_t>(p.first),8);b.Raw(std::bit_cast<std::uint64_t>(p.second),8);}if(r2000)b.Put(1,1);else b.Point({0,0,1});});
		xTemporaryFile file;file.Write(graph.Bytes());gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));gtl::dwg::sReadReport report;
		gtl::dwg::sShapeOptions options;options.hatchBoundaryOnly=true;auto drawing=gtl::dwg::ToShape(dwg,&report,options);REQUIRE(report.convertedEntities==1);
		auto const* poly=dynamic_cast<gtl::shape::xPolyline const*>(&drawing.m_layers.front().m_shapes.front());REQUIRE(poly);REQUIRE(poly->m_pts.size()==4);CHECK(poly->m_pts[2].x==2.);CHECK(poly->m_pts[2].y==2.);CHECK(poly->m_pts[2].z==3.);
		options.hatchBoundaryOnly=false;gtl::dwg::ToShape(dwg,&report,options);CHECK(report.convertedEntities==2);
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
		if (report.version >= gtl::dwg::eVERSION::r14) {
			REQUIRE(ok);
			CHECK_FALSE(dwg.GetDocument().objects.empty());
			CHECK(dwg.GetDocument().headerVariables.contains("LUNITS"));
			CHECK(dwg.GetDocument().headerVariables.contains("LTSCALE"));
			CHECK_FALSE(dwg.GetDocument().textStyles.empty());CHECK_FALSE(dwg.GetDocument().lineTypes.empty());
			if(report.version>=gtl::dwg::eVERSION::r2000)CHECK(dwg.GetDocument().insertionUnits.has_value());
			gtl::dwg::sReadReport converted;
			auto drawing = gtl::dwg::ToShape(dwg, &converted);
			if(report.version<=gtl::dwg::eVERSION::r2004)CHECK(converted.convertedEntities > 0);
			else CHECK((converted.convertedEntities>0 || !converted.diagnostics.empty()));
			CHECK_FALSE(drawing.m_layers.empty());
		}
		else {
			CHECK_FALSE(ok);
			CHECK(report.error == gtl::dwg::eREAD_ERROR::unsupported_version);
			CHECK(dwg.GetDocument().objects.empty());
		}
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

TEST_CASE("dwg modern split streams preserve Unicode and reject malformed strings", "[dwg][unit]") {
	using namespace gtl::dwg;
	for(auto [year,version,signature]:{
		std::tuple{2007u,eVERSION::r2007,"AC1021"},
		std::tuple{2010u,eVERSION::r2010,"AC1024"},
		std::tuple{2013u,eVERSION::r2013,"AC1027"},
		std::tuple{2018u,eVERSION::r2018,"AC1032"}}) {
		CAPTURE(year);
		auto decode=[&](auto const& sections) {
			sContainer container;container.version=version;
			for(auto const& [name,bytes]:sections)container.sections[name].data=bytes;
			return detail::ReadContainerObjects(container);
		};
		auto sections=fixture::UnicodeSections(year);
		auto doc=decode(sections);
		CHECK(doc.unicodeStrings);REQUIRE(doc.layers.size()==1);
		CHECK(doc.layers.at(1).name=="\xea\xb0\x80\xf0\x9f\x99\x82");
		CHECK(doc.layers.at(1).rgb==0x123456);REQUIRE(doc.entities.size()==1);
		auto const* circle=std::get_if<entities::sCircle>(&doc.entities.front().geometry);
		REQUIRE(circle);CHECK(circle->radius==5.);
		CHECK_THROWS_AS(decode(fixture::UnicodeSections(year,u"test",true)),detail::xParseError);
		std::u16string invalid{static_cast<char16_t>(0xd800),u'A'};
		CHECK_THROWS_AS(decode(fixture::UnicodeSections(year,invalid)),detail::xParseError);
		invalid={static_cast<char16_t>(0xdc00)};
		CHECK_THROWS_AS(decode(fixture::UnicodeSections(year,invalid)),detail::xParseError);
		if(year>=2010) {
			xTemporaryFile file;file.Write(fixture::PackSections(sections,signature));
			sReadReport report;auto drawing=ReadDWGShape(file.path,&report);INFO(report.message);
			REQUIRE(drawing);CHECK(report.convertedEntities==1);
			REQUIRE(drawing->m_layers.size()==1);CHECK(drawing->m_layers.front().m_name==L"\uac00\U0001f642");
			CHECK(report.diagnostics.empty());
		}
	}
}

TEST_CASE("dwg AC1018 object streams resolve explicit ownership and colors", "[dwg][unit]") {
	fixture::Graph graph;graph.modern=true;graph.Layer(1,"0",3,0x112233);graph.Layer(3,"target",5);
	SECTION("polyline explicit vertices") {
		graph.Polyline();
		xTemporaryFile file;file.Write(fixture::PackSections(graph.Sections()));
		gtl::dwg::xDWG dwg;auto ok=dwg.ReadDWG(file.path);INFO(dwg.GetReport().message);REQUIRE(ok);
		auto const& poly=std::get<gtl::dwg::entities::sPolyline>(dwg.GetDocument().entities.front().geometry);
		CHECK(poly.vertices==std::vector<gtl::dwg::handle_t>{4,7});REQUIRE(poly.points.size()==2);
		CHECK(poly.points[0].x==4.);CHECK(poly.points[1].x==7.);CHECK(poly.points[0].z==9.);CHECK(poly.bulges[0]==.5);
		gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);CHECK(report.convertedEntities==1);
	}
	SECTION("block ownership and entity true color") {
		graph.Block(10,20,20,{1,2,0});
		graph.Entity(0x12,20,10,[](fixture::Bits& b){b.Point({1,2,0});b.Double(2.);b.Put(3,2);},{},0,1,256,0xa1b2c3);
		graph.Insert(30,0,10,{10,20,0},{2,2,1},0,1,1,3);
		xTemporaryFile file;file.Write(fixture::PackSections(graph.Sections()));gtl::dwg::xDWG dwg;
		auto ok=dwg.ReadDWG(file.path);INFO(dwg.GetReport().message);REQUIRE(ok);
		CHECK(dwg.GetDocument().layers.at(1).rgb==0x112233);
		CHECK(dwg.GetDocument().blocks.at(10).entities==std::vector<gtl::dwg::handle_t>{20});
		gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);
		CHECK(report.diagnostics.empty());REQUIRE(report.convertedEntities==1);
		CHECK(drawing.m_layers[0].m_color==gtl::ColorRGBA(0x11,0x22,0x33));
		REQUIRE(drawing.m_layers[1].m_shapes.size()==1);
		auto const* circle=dynamic_cast<gtl::shape::xCircle const*>(&drawing.m_layers[1].m_shapes.front());REQUIRE(circle);
		CHECK(circle->m_color==gtl::ColorRGBA(0xa1,0xb2,0xc3));CHECK(circle->m_radius==4.);
		CHECK((circle->m_ptCenter==gtl::shape::point_t{10,20,0}));
		CHECK(gtl::dwg::ReadDWGShape(file.path).has_value());
	}
	SECTION("invalid owned vertex clears loaded state") {
		graph.Polyline();xTemporaryFile file;file.Write(fixture::PackSections(graph.Sections()));
		gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));
		graph.objects.erase(7);file.Write(fixture::PackSections(graph.Sections()));
		CHECK_FALSE(dwg.ReadDWG(file.path));CHECK(dwg.GetReport().error==gtl::dwg::eREAD_ERROR::invalid_data);
		CHECK_FALSE(dwg.IsLoaded());CHECK(dwg.GetDocument().entities.empty());
	}
	SECTION("object CRC is validated after successful decompression") {
		graph.Circle(2,0);auto sections=graph.Sections();sections.at("AcDb:AcDbObjects").back()^=1;
		xTemporaryFile file;file.Write(fixture::PackSections(sections));
		REQUIRE(gtl::dwg::ReadDWGContainer(file.path));gtl::dwg::xDWG dwg;CHECK_FALSE(dwg.ReadDWG(file.path));
		CHECK(dwg.GetReport().message.find("object CRC")!=std::string::npos);
	}
	SECTION("class versions occupy bitlongs") {
		graph.Circle(2,0);xTemporaryFile file;file.Write(fixture::PackSections(graph.Sections(true)));
		gtl::dwg::xDWG dwg;auto ok=dwg.ReadDWG(file.path);INFO(dwg.GetReport().message);REQUIRE(ok);
		REQUIRE(dwg.GetDocument().classes.size()==1);CHECK(dwg.GetDocument().classes.at(500).dxfName=="TEST_CLASS");
	}
	SECTION("INSERT explicit attribute ownership") {
		graph.Block(10,20,20);graph.Circle(20,10);
		graph.Entity(7,30,0,[](fixture::Bits& b){b.Point({0,0,0});b.Put(3,2);b.Double(0.);b.Point({0,0,1});b.Put(1,1);b.Long(2);},{10,31,32,33});
		auto attribute=[](fixture::Bits& b){b.Raw(255,1);b.Raw(std::bit_cast<std::uint64_t>(1.),8);b.Raw(std::bit_cast<std::uint64_t>(2.),8);b.Put(3,2);b.Raw(std::bit_cast<std::uint64_t>(2.),8);b.Text("value");b.Text("tag");b.Short(0);b.Raw(0,1);};
		graph.Entity(2,31,30,attribute,{0});graph.Entity(2,32,30,attribute,{0});graph.Entity(6,33,30,[](fixture::Bits&){});
		xTemporaryFile file;file.Write(fixture::PackSections(graph.Sections()));gtl::dwg::xDWG dwg;
		auto ok=dwg.ReadDWG(file.path);INFO(dwg.GetReport().message);REQUIRE(ok);
		gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);CHECK(report.convertedEntities==3);
		CHECK(report.diagnostics.empty());
	}
	SECTION("vertex list order is independent of numeric handles") {
		graph.Polyline();
		graph.Entity(0x0f,2,0,[](fixture::Bits& b){b.Short(1);b.Short(0);b.Double(0.);b.Double(0.);b.Put(1,1);b.Double(9.);b.Put(1,1);b.Long(2);},{7,4,8});
		xTemporaryFile file;file.Write(fixture::PackSections(graph.Sections()));gtl::dwg::xDWG dwg;REQUIRE(dwg.ReadDWG(file.path));
		auto const& poly=std::get<gtl::dwg::entities::sPolyline>(dwg.GetDocument().entities.front().geometry);
		REQUIRE(poly.points.size()==2);CHECK(poly.points[0].x==7.);CHECK(poly.points[1].x==4.);
	}
	SECTION("duplicate vertex references are rejected") {
		graph.Polyline();
		graph.Entity(0x0f,2,0,[](fixture::Bits& b){b.Short(1);b.Short(0);b.Double(0.);b.Double(0.);b.Put(1,1);b.Double(9.);b.Put(1,1);b.Long(2);},{4,4,8});
		xTemporaryFile file;file.Write(fixture::PackSections(graph.Sections()));gtl::dwg::xDWG dwg;CHECK_FALSE(dwg.ReadDWG(file.path));
		CHECK(dwg.GetReport().message.find("duplicate owned")!=std::string::npos);
	}
}

TEST_CASE("dwg face ray and shape definitions preserve geometry", "[dwg][unit]") {
 for (int revision : {0,1,2}) {
  fixture::Graph graph; graph.r2000=revision!=0; graph.modern=revision==2; graph.Layer(1,"0");
  graph.Entity(0x1c,2,0,[&](fixture::Bits& b) {
   if (!graph.r2000) for(auto p : {std::array<double,3>{1,2,3},{4,2,3},{4,5,3},{1,5,3}}) b.Point(p);
   else {
    b.Put(0,1);b.Put(0,1);for(double v:{1.,2.,3.})b.Raw(std::bit_cast<std::uint64_t>(v),8);
    for(double v:{4.,2.,3.,4.,5.,3.,1.,5.,3.}) {b.Put(3,2);b.Raw(std::bit_cast<std::uint64_t>(v),8);}
   }
   b.Short(2);
  });
  for(unsigned type:{0x28u,0x29u}) graph.Entity(type,type,0,[](fixture::Bits& b){b.Point({1,2,3});b.Point({0,1,0});});
  graph.Entity(0x21,3,0,[](fixture::Bits& b){b.Point({5,6,7});for(double v:{2.,.5,1.,0.,0.})b.Double(v);b.Short(42);b.Point({0,0,1});},{9});
  xTemporaryFile file;file.Write(graph.modern?fixture::PackSections(graph.Sections()):graph.Bytes());gtl::dwg::xDWG dwg;
  auto ok=dwg.ReadDWG(file.path);INFO(dwg.GetReport().message);REQUIRE(ok);
  auto const& entities=dwg.GetDocument().entities;
  auto const& face=std::get<gtl::dwg::entities::sFace3D>(entities[0].geometry);CHECK(face.corners[2].y==5);CHECK(face.invisibleEdges==2);
  auto const& shape=std::get<gtl::dwg::entities::sShape>(entities[1].geometry);CHECK(shape.number==42);CHECK(shape.style==9);
  CHECK_FALSE(std::get<gtl::dwg::entities::sRay>(entities[2].geometry).bothWays);CHECK(std::get<gtl::dwg::entities::sRay>(entities[3].geometry).bothWays);
  gtl::dwg::sReadReport report;auto drawing=gtl::dwg::ToShape(dwg,&report);CHECK(report.convertedEntities==3);CHECK(report.diagnostics.size()==4);
 }
}

TEST_CASE("dwg polyface and mesh validate ownership counts and signed indices", "[dwg][unit]") {
 for(int revision:{0,1,2}) for(bool mesh:{false,true}) for(bool invalid:{false,true}) {
  fixture::Graph graph;graph.r2000=revision!=0;graph.modern=revision==2;graph.Layer(1,"0");
  graph.Entity(mesh?0x1e:0x1d,2,0,[&](fixture::Bits& b){
   if(mesh){b.Short(0);b.Short(0);}b.Short(mesh?2:4);b.Short(mesh?2:1);
   if(mesh){b.Short(0);b.Short(0);}if(graph.modern)b.Long(mesh?4:5);
  },graph.modern?(mesh?std::vector<unsigned>{4,5,6,7,9}:std::vector<unsigned>{4,5,6,7,8,9}):std::vector<unsigned>{4,mesh?7u:8u,9});
  for(unsigned i=4;i<=7;++i) graph.Entity(mesh?0x0c:0x0d,i,(invalid&&mesh&&i==7)?0:2,[&](fixture::Bits& b){b.Raw(0,1);b.Point({double(i),0,0});},{},i==7?(mesh?0:8):i+1);
  if(!mesh)graph.Entity(0x0e,8,2,[&](fixture::Bits& b){b.Short(1);b.Short(-2);b.Short(invalid?5:3);b.Short(4);});
  graph.Entity(6,9,2,[](fixture::Bits&){});
  xTemporaryFile file;file.Write(graph.modern?fixture::PackSections(graph.Sections()):graph.Bytes());gtl::dwg::xDWG dwg;
  auto ok=dwg.ReadDWG(file.path);INFO(dwg.GetReport().message);REQUIRE(ok==!invalid);
  if(!ok)continue;
  auto const& poly=std::get<gtl::dwg::entities::sPolyline>(dwg.GetDocument().entities.front().geometry);CHECK(poly.points.size()==4);
  if(!mesh){REQUIRE(poly.faces.size()==1);CHECK(poly.faces[0].indices[1]==-2);}
  gtl::dwg::sReadReport report;gtl::dwg::ToShape(dwg,&report);CHECK(report.convertedEntities==0);CHECK_FALSE(report.diagnostics.empty());
 }
}

TEST_CASE("dwg new primitive decoders consume modern split streams", "[dwg][unit]") {
 for(unsigned year:{2007u,2010u,2013u,2018u})for(unsigned type:{0x1cu,0x21u,0x28u,0x29u}) {
  CAPTURE(year,type);
  gtl::dwg::sContainer container;container.version=year==2007?gtl::dwg::eVERSION::r2007:year==2010?gtl::dwg::eVERSION::r2010:year==2013?gtl::dwg::eVERSION::r2013:gtl::dwg::eVERSION::r2018;
  auto geometry=[&](fixture::Bits& b){
   if(type==0x1c){b.Put(1,1);b.Put(1,1);b.Raw(0,8);b.Raw(0,8);for(double v:{1.,0.,0.,1.,1.,0.,0.,1.,0.}){b.Put(3,2);b.Raw(std::bit_cast<std::uint64_t>(v),8);}}
   else if(type==0x21){b.Point({1,2,3});for(double v:{1.,0.,1.,0.,0.})b.Double(v);b.Short(5);b.Point({0,0,1});}
   else{b.Point({1,2,3});b.Point({0,1,0});}
  };
  for(auto const& [name,bytes]:fixture::UnicodeSections(year,u"test",false,type,geometry,{},type==0x21?std::vector<unsigned>{9}:std::vector<unsigned>{}))container.sections[name].data=bytes;
  auto doc=gtl::dwg::detail::ReadContainerObjects(container);REQUIRE(doc.entities.size()==1);
  if(type==0x1c){auto const& f=std::get<gtl::dwg::entities::sFace3D>(doc.entities[0].geometry);CHECK(f.invisibleEdges==0);CHECK(f.corners[2].y==1);}
  else if(type==0x21)CHECK(std::get<gtl::dwg::entities::sShape>(doc.entities[0].geometry).style==9);
  else CHECK(std::get<gtl::dwg::entities::sRay>(doc.entities[0].geometry).bothWays==(type==0x29));
 }
 fixture::Graph graph;graph.Layer(1,"0");graph.Entity(0x28,2,0,[](fixture::Bits& b){b.Point({0,0,0});b.Point({0,0,0});});
 xTemporaryFile file;file.Write(graph.Bytes());gtl::dwg::xDWG dwg;CHECK_FALSE(dwg.ReadDWG(file.path));
}

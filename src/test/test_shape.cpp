#include "pch.h"

#include "gtl/gtl.h"

#include "gtl/shape/shape.h"


using namespace std::literals;
using namespace gtl::literals;

TEST(gtl_shape, basic) {
	gtl::bjson jDXF;
	jDXF.read(LR"(D:\Project\APS\InkPatternGenerator\samples\Cell 잉크젯 Test_dxf2007.dxf.json)");
	//{
	//	auto j = jDXF.json();
	//	//auto& jObject = j.as_object();
	//	//auto& header = jObject["header"];
	//	auto jVars = jDXF["header"]["vars"];
	//	for (auto var : jVars.json().as_object()) {
	//		auto key = var.key();
	//		auto v = var.value();
	//	}
	//	
	//	//auto& varsObject = header.as_object();
	//	//auto& varsArray = varsObject;
	//	//for (auto const& i : jObject) {
	//	//	auto key = i.key();
	//	//	auto v = i.value();
	//	//}

	//}

	gtl::shape::s_drawing cad;

	std::wstring strLayer;
	cad.FromJson(jDXF.json(), strLayer);

}


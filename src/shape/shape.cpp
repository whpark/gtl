#include "pch.h"
#include <functional>
#include "gtl/shape/shape.h"

using namespace std::literals;

BOOST_CLASS_EXPORT_GUID(gtl::shape::s_layer, "layer")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_dot, "dot")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_line, "line")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_polyline, "polyline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_circleXY, "circleXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_arcXY, "arcXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_ellipseXY, "ellipseXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_spline, "spline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_text, "text")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_mtext, "mtext")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_hatch, "hatch")

namespace gtl::shape {

	bool s_shape::FromJson(json_t& _j, string_t& layer) {
		using namespace std::literals;
		gtl::bjson<json_t> j(_j);
		//color.cr = (int)j["color"sv];
		layer = j["layer"sv];

		lineWeight = j["lWeight"];
		strLineType = j["lineType"];

		//eLineType = j["lineType"sv];
		crIndex = j["color"].value_or(0);
		if (j["color24"].json().is_int64()) {
			crIndex = 0;
			color.cr = (int)j["color24"];
		} else {
			if (crIndex == 256) {
				// todo :...
			} else if ( (crIndex >= 0) and (crIndex < colorTable_s.size()) )
				color = colorTable_s[crIndex];
		}
		bVisible = j["visible"].value_or(true);
		bTransparent = j["transparency"].value_or(0) != false;

		return true;
	}


	std::unique_ptr<s_shape> s_shape::CreateShapeFromName(std::string const& strEntityName) {
		static std::map<std::string, std::function<std::unique_ptr<s_shape>()> > const mapCreator = {
			//{ "3dFace"s,				nullptr },
			{ "ARC"s,					[](){ return std::make_unique<s_arcXY>(); } },
			//{ "BLOCK"s,					nullptr },
			{ "CIRCLE"s,				[](){ return std::make_unique<s_circleXY>(); } },
			{ "DIMENSION"s,				nullptr },
			{ "DIMALIGNED"s,			nullptr },
			{ "DIMLINEAR"s,				nullptr },
			{ "DIMRADIAL"s,				nullptr },
			{ "DIMDIAMETRIC"s,			nullptr },
			{ "DIMANGULAR"s,			nullptr },
			{ "DIMANGULAR3P"s,			nullptr },
			{ "DIMORDINATE"s,			nullptr },
			{ "ELLIPSE"s,				[](){ return std::make_unique<s_ellipseXY>(); } },
			{ "HATCH"s,					[](){ return std::make_unique<s_hatch>(); } },
			{ "IMAGE"s,					nullptr },
			{ "INSERT"s,				[](){ return std::make_unique<s_insert>(); } },
			{ "LEADER"s,				nullptr },
			{ "LINE"s,					[](){ return std::make_unique<s_line>(); } },
			{ "LWPOLYLINE"s,			[](){ return std::make_unique<s_lwpolyline>(); } },
			{ "MTEXT"s,					[](){ return std::make_unique<s_mtext>(); } },
			{ "POINT"s,					[](){ return std::make_unique<s_dot>(); } },
			{ "POLYLINE"s,				[](){ return std::make_unique<s_polyline>(); } },
			{ "RAY"s,					nullptr },
			{ "SOLID"s,					nullptr },
			{ "SPLINE"s,				[](){ return std::make_unique<s_spline>(); } },
			{ "TEXT"s,					[](){ return std::make_unique<s_text>(); } },
			{ "TRACE"s,					nullptr },
			{ "UNDERLAY"s,				nullptr },
			{ "VERTEX"s,				nullptr },
			{ "VIEWPORT"s,				nullptr },
			{ "XLINE"s,					nullptr },
		};

		auto iter = mapCreator.find(strEntityName);
		if (iter == mapCreator.end())
			return {};
		auto& creator = iter->second;
		if (creator)
			return creator();
		return {};
	}

	void s_polyline::Draw(ICanvas& canvas) const {
		for (int iPt = 0; iPt < pts.size(); iPt++) {
			polypoint_t pt0{pts[iPt]};
			polypoint_t pt1;
			bool bLast = false;
			if (bLoop) {
				if (iPt == pts.size()-1) {
					pt1 = pts[0];
					bLast = true;
				} else {
					pt1 = pts[iPt+1];
				}
			} else {
				if (iPt == pts.size()-1)
					break;
				bLast = iPt == pts.size()-2;
				pt1 = pts[iPt+1];
			}

			double dBulge = pt0.Bulge();

			if (dBulge == 0.0) {
				canvas.LineTo(pt1);
			} else {
				canvas.LineTo(pt0);
				s_arcXY arc = s_arcXY::GetFromBulge(dBulge, pt0, pt1);
				(s_shape&)arc = *(s_shape*)this;

				arc.Draw(canvas);

				canvas.LineTo(pt1);
			}
		}
	}

	bool s_drawing::FromJson(json_t& _j, string_t& layer) {
		//s_shape::FromJson(_j, layer);

		gtl::bjson<json_t> jTOP(_j);

		// header
		{
			auto jHeader = jTOP["header"];
			auto jVars = jHeader["vars"].json().as_object();
			vars.clear();
			for (auto& var : jVars) {
				auto key = var.key();
				auto value = var.value();
				if (value.is_string()) {
					auto& str = value.as_string();
					vars[std::string(key.data(), key.size())] = gtl::ToString<gtl::shape::char_t, char>(std::string{str.data(), str.size()});
				} else if (value.is_int64()) {
					vars[std::string(key.data(), key.size())] = (int)value.as_int64();
				} else if (value.is_double()) {
					vars[std::string(key.data(), key.size())] = value.as_double();
				}
			}
		}

		// line types
		{
			line_types.clear();
			auto jLineTypes = jTOP["lineTypes"].json().as_array();
			for (auto& item : jLineTypes) {
				bjson<json_t> jLT(item);
				auto lt = std::make_unique<line_type_t>();
				lt->name = jLT["name"];
				lt->flags = jLT["flags"];
				lt->description = jLT["desc"];
				if (jLT["path"].json().is_array()) {
					for (auto& jp : jLT["path"].json().as_array()) {
						lt->path.push_back(jp.is_double() ? jp.as_double() : jp.as_int64());
					}
				}
				line_types.push_back(std::move(lt));
			}
		}

		// layers
		std::map<string_t, s_layer*> mapLayers;	// cache
		{
			layers.clear();
			//layers.push_back(std::make_unique<s_layer>(L"0"));

			auto jLayers = jTOP["layers"].json().as_array();
			for (auto& item : jLayers) {
				bjson<json_t> j(item);

				auto rLayer = std::make_unique<s_layer>();
				string_t strLayer;
				rLayer->FromJson(item, strLayer);

				if (auto iterLineType = std::find_if(line_types.begin(), line_types.end(), [&rLayer](auto const& lt) { return lt.name == rLayer->strLineType; });
					iterLineType != line_types.end()) {

					rLayer->pLineType = &(*iterLineType);
				}

				mapLayers[rLayer->name] = rLayer.get();
				layers.push_back(std::move(rLayer));
			}
		}

		// block
		boost::ptr_deque<s_block> blocks;
		std::map<string_t, s_block*> mapBlocks;
		{
			blocks.clear();
			auto jBlocks = jTOP["blocks"].json().as_array();
			for (auto& item : jBlocks) {
				bjson<json_t> j(item);
				auto rBlock = std::make_unique<s_block>();

				string_t strDummy;
				rBlock->FromJson(item, strDummy);

				if (auto iterLineType = std::find_if(line_types.begin(), line_types.end(), [&rBlock](auto const& lt) { return lt.name == rBlock->strLineType; });
					iterLineType != line_types.end()) {

					rBlock->pLineType = &(*iterLineType);
				}

				// block entities
				for (auto& jEntity : j["entities"].json().as_array()) {
					bjson<json_t> j(jEntity);
					std::string strEntityName = j["entityName"];

					std::unique_ptr<s_shape> rShape = CreateShapeFromName(strEntityName);
					if (!rShape)
						continue;

					string_t strLayer;
					rShape->FromJson(jEntity, strLayer);
					switch (rShape->GetType()) {
					case eTYPE::insert :
						break;
					}

					rBlock->shapes.push_back(std::move(rShape));
				}

				mapBlocks[rBlock->name] = rBlock.get();
				blocks.push_back(std::move(rBlock));
			}
		}


		// Entities
		{
			auto jEntities = jTOP["mainBlock"].json().as_array();

			// block entities
			for (auto& jEntity : jEntities) {
				bjson<json_t> j(jEntity);
				std::string strEntityName = j["entityName"];

				std::unique_ptr<s_shape> rShape = CreateShapeFromName(strEntityName);
				if (!rShape)
					continue;

				string_t strLayer;
				rShape->FromJson(jEntity, strLayer);

				auto* pLayer = mapLayers[strLayer];
				if (!pLayer)
					continue;

				switch (rShape->GetType()) {
				case eTYPE::insert :
					// todo :
					if (s_insert* pInsert = dynamic_cast<s_insert*>(rShape.get()); pInsert) {
						if (auto* pBlock = mapBlocks[pInsert->name]; !pBlock) {
							// todo : position, scale...
							rShape = pBlock->NewClone();
						}
					}
					break;
				}

				pLayer->shapes.push_back(std::move(rShape));

			}
		}

		return true;
	}


};

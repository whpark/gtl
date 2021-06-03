#include "pch.h"
#include <functional>
#include "gtl/shape/shape.h"

using namespace std::literals;

#ifdef _DEBUG
#define DEBUG_PRINT(...) fmt::print(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) 
#endif

namespace gtl::shape {

	bool s_shape::LoadFromCADJson(json_t& _j) {
		using namespace std::literals;
		gtl::bjson<json_t> j(_j);
		//color.cr = (int)j["color"sv];
		strLayer = j["layer"sv];

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

	string_t const& s_shape::GetShapeName(eSHAPE eType) {
		static std::map<eSHAPE, string_t> const map = {
			{ eSHAPE::none,				L"none" },
			{ eSHAPE::e3dface,			L"3dFace"s },
			{ eSHAPE::arc_xy,			L"ARC"s },
			{ eSHAPE::block,			L"BLOCK"s },
			{ eSHAPE::circle_xy,		L"CIRCLE"s },
			{ eSHAPE::dimension,		L"DIMENSION"s },
			{ eSHAPE::dimaligned,		L"DIMALIGNED"s },
			{ eSHAPE::dimlinear,		L"DIMLINEAR"s },
			{ eSHAPE::dimradial,		L"DIMRADIAL"s },
			{ eSHAPE::dimdiametric,		L"DIMDIAMETRIC"s },
			{ eSHAPE::dimangular,		L"DIMANGULAR"s },
			{ eSHAPE::dimangular3p,		L"DIMANGULAR3P"s },
			{ eSHAPE::dimordinate,		L"DIMORDINATE"s },
			{ eSHAPE::ellipse_xy,		L"ELLIPSE"s },
			{ eSHAPE::hatch,			L"HATCH"s },
			{ eSHAPE::image,			L"IMAGE"s },
			{ eSHAPE::insert,			L"INSERT"s },
			{ eSHAPE::leader,			L"LEADER"s },
			{ eSHAPE::line,				L"LINE"s },
			{ eSHAPE::lwpolyline,		L"LWPOLYLINE"s },
			{ eSHAPE::mtext,			L"MTEXT"s },
			{ eSHAPE::dot,				L"POINT"s },
			{ eSHAPE::polyline,			L"POLYLINE"s },
			{ eSHAPE::ray,				L"RAY"s },
			{ eSHAPE::solid,			L"SOLID"s },
			{ eSHAPE::spline,			L"SPLINE"s },
			{ eSHAPE::text,				L"TEXT"s },
			{ eSHAPE::trace,			L"TRACE"s },
			{ eSHAPE::underlay,			L"UNDERLAY"s },
			{ eSHAPE::vertex,			L"VERTEX"s },
			{ eSHAPE::viewport,			L"VIEWPORT"s },
			{ eSHAPE::xline,			L"XLINE"s },
			{ eSHAPE::layer,			L"LAYER"s },
			{ eSHAPE::drawing,			L"DRAWING"s },
		};

		auto iter = map.find(eType);
		if (iter == map.end())
			return L""s;
		return iter->second;
	}

	std::unique_ptr<s_shape> s_shape::CreateShapeFromEntityName(std::string const& strEntityName) {
		static std::map<std::string, std::function<std::unique_ptr<s_shape>()> > const mapCreator = {
			{ "3dFace"s,				nullptr },
			{ "ARC"s,					[](){ return std::make_unique<s_arcXY>(); } },
			{ "BLOCK"s,					nullptr },
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

	bool s_drawing::LoadFromCADJson(json_t& _j) {
		//s_shape::LoadFromCADJson(_j);

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
				rLayer->LoadFromCADJson(item);

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

				try {
					rBlock->LoadFromCADJson(item);
				} catch (std::exception& e) {
					DEBUG_PRINT("{}\n", e.what());
					continue;
				} catch (...) {
					DEBUG_PRINT("unknown\n");
					continue;
				}


				if (auto iterLineType = std::find_if(line_types.begin(), line_types.end(), [&rBlock](auto const& lt) { return lt.name == rBlock->strLineType; });
					iterLineType != line_types.end()) {

					rBlock->pLineType = &(*iterLineType);
				}

				// block entities
				for (auto& jEntity : j["entities"].json().as_array()) {
					bjson<json_t> j(jEntity);
					std::string strEntityName = j["entityName"];

					std::unique_ptr<s_shape> rShape = CreateShapeFromEntityName(strEntityName);
					if (!rShape)
						continue;

					try {
						rShape->LoadFromCADJson(jEntity);
					} catch (std::exception& e) {
						DEBUG_PRINT("{}\n", e.what());
						continue;
					} catch (...) {
						DEBUG_PRINT("unknown\n");
						continue;
					}

					switch (rShape->GetShapeType()) {
					case eSHAPE::insert :
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

				std::unique_ptr<s_shape> rShape = CreateShapeFromEntityName(strEntityName);
				if (!rShape)
					continue;

				try {
					rShape->LoadFromCADJson(jEntity);
				} catch (std::exception& e) {
					DEBUG_PRINT("{}\n", e.what());
					continue;
				} catch (...) {
					DEBUG_PRINT("unknown\n");
					continue;
				}

				AddEntity(std::move(rShape), mapLayers, mapBlocks);
			}
		}

		return true;
	}

	bool s_drawing::AddEntity(std::unique_ptr<s_shape> rShape, std::map<string_t, s_layer*> const& mapLayers, std::map<string_t, s_block*> const& mapBlocks) {
		if (!rShape)
			return false;

		switch (rShape->GetShapeType()) {
		case eSHAPE::insert :
			// todo :
			if (s_insert* pInsert = dynamic_cast<s_insert*>(rShape.get()); pInsert) {
				auto iter = mapBlocks.find(pInsert->name);
				if (iter == mapBlocks.end()) {
					DEBUG_PRINT(L"No Block : {}\n", pInsert->name);
					return false;
				}
				auto* pBlock = iter->second;
				if (!pBlock) {
					DEBUG_PRINT(L"Internal ERROR (Block Name : {})\n", pInsert->name);
					return false;
				}

				ct_t ct;
				// todo : 순서 확인 (scale->rotate ? or rotate->scale ?)
				if (pInsert->xscale != 1.0) {
					ct.mat_(0, 0) *= pInsert->xscale;
					ct.mat_(0, 1) *= pInsert->xscale;
					ct.mat_(0, 2) *= pInsert->xscale;
				}
				if (pInsert->yscale != 1.0) {
					ct.mat_(1, 0) *= pInsert->yscale;
					ct.mat_(1, 1) *= pInsert->yscale;
					ct.mat_(1, 2) *= pInsert->yscale;
				}
				if (pInsert->zscale != 1.0) {
					ct.mat_(2, 0) *= pInsert->zscale;
					ct.mat_(2, 1) *= pInsert->zscale;
					ct.mat_(2, 2) *= pInsert->zscale;
				}

				if (pInsert->angle != 0.0_rad) {
					ct.mat_ = ct.GetRotatingMatrixXY(pInsert->angle) * ct.mat_;
				}

				ct.origin_ = pBlock->pt;

				for (int y = 0; y < pInsert->nRow; y++) {
					for (int x = 0; x < pInsert->nCol; x++) {

						std::unique_ptr<s_block> rBlockNew { dynamic_cast<s_block*>(pBlock->NewClone().release()) };

						ct.offset_.x = x*pInsert->spacingCol + pInsert->pt.x;
						ct.offset_.y = y*pInsert->spacingRow + pInsert->pt.x;

						for (auto const& rShape : rBlockNew->shapes) {
							auto rShapeNew = rShape.NewClone();
							rBlockNew->Transform(ct, ct.IsRightHanded());
							if (!AddEntity(std::move(rShapeNew), mapLayers, mapBlocks)) {
								DEBUG_PRINT("CANNOT Add Shape\n");
								//return false;
							}
						}
					}
				}
			}
			break;

		default :
			if (auto iter = mapLayers.find(rShape->strLayer); iter != mapLayers.end()) {
				auto* pLayer = iter->second;
				if (!pLayer) {
					DEBUG_PRINT(L"Internal Error : Layer {}\n", rShape->strLayer);
					return false;
				}

				pLayer->shapes.push_back(std::move(rShape));

			} else {
				DEBUG_PRINT(L"No Layer : {}\n", rShape->strLayer);
				return false;
			}

			break;
		}

		return true;
	}


};

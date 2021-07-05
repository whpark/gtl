module;

#include <array>
#include <cstdint>
#include <optional>
#include <vector>
#include <deque>
#include <string>
#include <string_view>

#include "gtl/gtl.h"

#include "gtl/_config.h"
#include "gtl/_macro.h"

#include "opencv2/opencv.hpp"

#include "fmt/format.h"

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

export module gtls;
import gtl;
export import :color_table;
export import :primitives;
export import :others;
export import :canvas;

export {
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xShape, "shape")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xLayer, "layer")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xDot, "dot")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xLine, "line")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xPolyline, "polyline")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xPolylineLW, "lwpolyline")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xCircle, "circleXY")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xArc, "arcXY")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xEllipse, "ellipseXY")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xSpline, "spline")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xText, "text")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xMText, "mtext")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xHatch, "hatch")
	BOOST_CLASS_EXPORT_GUID(gtl::shape::xDrawing, "drawing")

}

#ifdef _DEBUG
#define DEBUG_PRINT(...) fmt::print(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) 
#endif

namespace gtl::shape {
	using namespace std::literals;
	using namespace gtl::literals;

}
namespace gtl {
	//template < typename archive >
	//void serialize(archive& ar, color_rgba_t& cr, unsigned int const file_version) {
	//	ar & cr.cr;
	//}
}

export namespace gtl::shape {

	void xShape::Draw(ICanvas& canvas) const {
		canvas.PreDraw(*this);
	}
	bool xShape::DrawROI(ICanvas& canvas, rect_t const& rectROI) const {
		rect_t rectBoundary;
		rectBoundary.SetRectEmptyForMinMax2d();
		UpdateBoundary(rectBoundary);
		if (!rectBoundary.IntersectRect(rectROI).IsRectHavingLength2d())
			return false;
		Draw(canvas);
		return true;
	}

	bool xShape::LoadFromCADJson(json_t& _j) {
		using namespace std::literals;
		gtl::bjson<json_t> j(_j);
		//color.cr = (int)j["color"sv];
		strLayer = j["layer"sv];

		lineWeight = j["lWeight"];
		strLineType = j["lineType"];

		//eLineType = j["lineType"sv];
		crIndex = j["color"].value_or(0);
		color.cr = -1;
		if (j["color24"].json().is_int64()) {
			crIndex = 0;
			color.cr = (int)j["color24"];
		} else {
			if ( (crIndex > 0) and (crIndex < colorTable_s.size()) )
				color = colorTable_s[crIndex];
		}
		bVisible = j["visible"].value_or(true);
		bTransparent = j["transparency"].value_or(0) != false;

		return true;
	}

	string_t const& xShape::GetShapeName(eSHAPE eType) {
		using namespace std::literals;
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
		if (iter == map.end()) {
			static auto const empty = L""s;
			return empty;
		}
		return iter->second;
	}

	std::unique_ptr<xShape> xShape::CreateShapeFromEntityName(std::string const& strEntityName) {
		using namespace std::literals;
		static std::map<std::string, std::function<std::unique_ptr<xShape>()> > const mapCreator = {
			{ "3dFace"s,				nullptr },
			{ "ARC"s,					[](){ return std::make_unique<xArc>(); } },
			{ "BLOCK"s,					nullptr },
			{ "CIRCLE"s,				[](){ return std::make_unique<xCircle>(); } },
			{ "DIMENSION"s,				nullptr },
			{ "DIMALIGNED"s,			nullptr },
			{ "DIMLINEAR"s,				nullptr },
			{ "DIMRADIAL"s,				nullptr },
			{ "DIMDIAMETRIC"s,			nullptr },
			{ "DIMANGULAR"s,			nullptr },
			{ "DIMANGULAR3P"s,			nullptr },
			{ "DIMORDINATE"s,			nullptr },
			{ "ELLIPSE"s,				[](){ return std::make_unique<xEllipse>(); } },
			{ "HATCH"s,					[](){ return std::make_unique<xHatch>(); } },
			{ "IMAGE"s,					nullptr },
			{ "INSERT"s,				[](){ return std::make_unique<s_insert>(); } },
			{ "LEADER"s,				nullptr },
			{ "LINE"s,					[](){ return std::make_unique<xLine>(); } },
			{ "LWPOLYLINE"s,			[](){ return std::make_unique<xPolylineLW>(); } },
			{ "MTEXT"s,					[](){ return std::make_unique<xMText>(); } },
			{ "POINT"s,					[](){ return std::make_unique<xDot>(); } },
			{ "POLYLINE"s,				[](){ return std::make_unique<xPolyline>(); } },
			{ "RAY"s,					nullptr },
			{ "SOLID"s,					nullptr },
			{ "SPLINE"s,				[](){ return std::make_unique<xSpline>(); } },
			{ "TEXT"s,					[](){ return std::make_unique<xText>(); } },
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

	void xPolyline::Draw(ICanvas& canvas) const {
		xShape::Draw(canvas);

		if (pts.size())
			canvas.MoveTo(pts[0]);

		auto nPt = pts.size();
		if (!bLoop)
			nPt--;
		for (int iPt = 0; iPt < nPt; iPt++) {
			auto iPt2 = (iPt+1) % pts.size();
			auto pt0 = pts[iPt];
			auto pt1 = pts[iPt2];
			if (pt0.Bulge() == 0.0) {
				canvas.LineTo(pt1);
			} else {
				canvas.LineTo(pt0);
				xArc arc = xArc::GetFromBulge(pt0.Bulge(), pt0, pt1);
				(xShape&)arc = *(xShape*)this;

				arc.Draw(canvas);

				canvas.LineTo(pt1);
			}
		}
	}

	boost::ptr_deque<xShape> xPolyline::Split() const {
		boost::ptr_deque<xShape> shapes;

		auto nPt = pts.size();
		if (!bLoop)
			nPt--;
		for (int iPt = 0; iPt < nPt; iPt++) {
			auto iPt2 = (iPt+1) % pts.size();
			auto pt0 = pts[iPt];
			auto pt1 = pts[iPt2];
			if (pt0.Bulge() == 0.0) {
				auto rLine = std::make_unique<xLine>();
				(xShape&)*rLine = (xShape const&)*this;
				rLine->pt0 = pt0;
				rLine->pt1 = pt1;
				shapes.push_back(std::move(rLine));
			} else {
				xArc arc = xArc::GetFromBulge(pt0.Bulge(), pt0, pt1);
				(xShape&)arc = *(xShape*)this;
				shapes.push_back(arc.NewClone());
			}
		}
		return shapes;
	}

	bool xDrawing::LoadFromCADJson(json_t& _j) {
		//xShape::LoadFromCADJson(_j);

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
					vars[std::string(key.data(), key.size())] = gtl::ToString<gtl::shape::char_t, char8_t>(std::u8string{(char8_t const*)str.data(), str.size()});
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
		std::map<string_t, xLayer*> mapLayers;	// cache
		{
			layers.clear();
			//layers.push_back(std::make_unique<xLayer>(L"0"));

			auto jLayers = jTOP["layers"].json().as_array();
			for (auto& item : jLayers) {
				bjson<json_t> j(item);

				auto rLayer = std::make_unique<xLayer>();
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
					e;
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

					std::unique_ptr<xShape> rShape = CreateShapeFromEntityName(strEntityName);
					if (!rShape)
						continue;

					try {
						rShape->LoadFromCADJson(jEntity);
					} catch (std::exception& e) {
						e;
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
			rectBoundary.SetRectEmptyForMinMax2d();
			// block entities
			for (auto& jEntity : jEntities) {
				bjson<json_t> j(jEntity);
				std::string strEntityName = j["entityName"];

				std::unique_ptr<xShape> rShape = CreateShapeFromEntityName(strEntityName);
				if (!rShape)
					continue;

				try {
					rShape->LoadFromCADJson(jEntity);
					//rShape->UpdateBoundary(rectBoundary);
				} catch (std::exception& e) {
					e;
					DEBUG_PRINT("{}\n", e.what());
					continue;
				} catch (...) {
					DEBUG_PRINT("unknown\n");
					continue;
				}

				AddEntity(std::move(rShape), mapLayers, mapBlocks, rectBoundary);
			}
		}

		return true;
	}

	bool xDrawing::AddEntity(std::unique_ptr<xShape> rShape, std::map<string_t, xLayer*> const& mapLayers, std::map<string_t, s_block*> const& mapBlocks, rect_t& rectB) {
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

				xCoordTrans3d ct;
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
							// color
							if (rShapeNew->crIndex == 0) {
								rShapeNew->crIndex = pBlock->crIndex;
							}
							// line Width
							if (rShapeNew->lineWeight == (int)eLINE_WIDTH::ByBlock) {
								rShapeNew->lineWeight = pBlock->lineWeight;
							}
							if (!AddEntity(std::move(rShapeNew), mapLayers, mapBlocks, rectB)) {
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

				// color
				if (rShape->color.cr == -1) {
					int crIndex = rShape->crIndex;
					if (crIndex == 256) {
						crIndex = pLayer->crIndex;
						rShape->color = pLayer->color;
					}
					if (rShape->color.cr == -1) {
						if ( (crIndex > 0) and (crIndex < colorTable_s.size()) )
							rShape->color = colorTable_s[crIndex];
					}
				}

				// line Width
				if (rShape->lineWeight == (int)eLINE_WIDTH::ByLayer) {
					rShape->lineWeight = pLayer->lineWeight;
				}

				rShape->UpdateBoundary(rectB);
				pLayer->shapes.push_back(std::move(rShape));

			} else {
				DEBUG_PRINT(L"No Layer : {}\n", rShape->strLayer);
				return false;
			}

			break;
		}

		return true;
	}


}

export namespace gtls = gtl::shape;

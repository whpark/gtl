﻿#include "pch.h"
#include <functional>
#include "gtl/shape/shape.h"

using namespace std::literals;

#ifdef _DEBUG
#define DEBUG_PRINT(...) fmt::print(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) 
#endif

namespace gtl::shape {

	//// from openCV
	//bool clipLine(gtl::CSize2i size, gtl::CPoint2d& pt1, gtl::CPoint2d& pt2) {
	//	if( size.cx <= 0 || size.cy <= 0 )
	//		return false;

	//	auto right = size.cx-1, bottom = size.cy-1;
	//	auto &x1 = pt1.x, &y1 = pt1.y, &x2 = pt2.x, &y2 = pt2.y;

	//	int c1 = (x1 < 0) + (x1 > right) * 2 + (y1 < 0) * 4 + (y1 > bottom) * 8;
	//	int c2 = (x2 < 0) + (x2 > right) * 2 + (y2 < 0) * 4 + (y2 > bottom) * 8;

	//	if( (c1 & c2) == 0 && (c1 | c2) != 0 )
	//	{
	//		int64 a;
	//		if( c1 & 12 )
	//		{
	//			a = c1 < 8 ? 0 : bottom;
	//			x1 += (int64)((double)(a - y1) * (x2 - x1) / (y2 - y1));
	//			y1 = a;
	//			c1 = (x1 < 0) + (x1 > right) * 2;
	//		}
	//		if( c2 & 12 )
	//		{
	//			a = c2 < 8 ? 0 : bottom;
	//			x2 += (int64)((double)(a - y2) * (x2 - x1) / (y2 - y1));
	//			y2 = a;
	//			c2 = (x2 < 0) + (x2 > right) * 2;
	//		}
	//		if( (c1 & c2) == 0 && (c1 | c2) != 0 )
	//		{
	//			if( c1 )
	//			{
	//				a = c1 == 1 ? 0 : right;
	//				y1 += (int64)((double)(a - x1) * (y2 - y1) / (x2 - x1));
	//				x1 = a;
	//				c1 = 0;
	//			}
	//			if( c2 )
	//			{
	//				a = c2 == 1 ? 0 : right;
	//				y2 += (int64)((double)(a - x2) * (y2 - y1) / (x2 - x1));
	//				x2 = a;
	//				c2 = 0;
	//			}
	//		}

	//		//assert( (c1 & c2) != 0 || (x1 | y1 | x2 | y2) >= 0 );
	//	}

	//	return (c1 | c2) == 0;
	//}

	// Cohen–Sutherland clipping algorithm clips a line from
	// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
	// diagonal from (xmin, ymin) to (xmax, ymax).
	void CohenSutherlandLineClip(gtl::CRect2d roi, gtl::CPoint2d& pt0, gtl::CPoint2d& pt1) {
		enum fOUT_CODE : uint8_t {
			INSIDE	= 0b0000,
			LEFT	= 0b0001,
			RIGHT	= 0b0010,
			BOTTOM	= 0b0100,
			TOP		= 0b1000,
		};

		auto& x0 = pt0.x;
		auto& y0 = pt0.y;
		auto& x1 = pt1.x;
		auto& y1 = pt1.y;

		// Compute the bit code for a point (x, y) using the clip
		// bounded diagonally by (xmin, ymin), and (xmax, ymax)
		auto ComputeOutCode = [&roi](auto x, auto y) -> fOUT_CODE {
			fOUT_CODE code {};
			if (x < roi.left)
				(uint8_t&)code |= fOUT_CODE::LEFT;
			else if (x >= roi.right)
				(uint8_t&)code |= fOUT_CODE::RIGHT;
			if (y < roi.top)
				(uint8_t&)code |= fOUT_CODE::BOTTOM;
			else if (y >= roi.bottom)
				(uint8_t&)code |= fOUT_CODE::TOP;
			return code;
		};

		// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
		auto outcode0 = ComputeOutCode(x0, y0);
		auto outcode1 = ComputeOutCode(x1, y1);
		bool accept = false;

		while (true) {
			if (!(outcode0 | outcode1)) {
				// bitwise OR is 0: both points inside window; trivially accept and exit loop
				accept = true;
				break;
			} else if (outcode0 & outcode1) {
				// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
				// or BOTTOM), so both must be outside window; exit loop (accept is false)
				break;
			} else {
				// failed both tests, so calculate the line segment to clip
				// from an outside point to an intersection with clip edge
				double x, y;

				// At least one endpoint is outside the clip rectangle; pick it.
				auto outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

				// Now find the intersection point;
				// use formulas:
				//   slope = (y1 - y0) / (x1 - x0)
				//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
				//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
				// No need to worry about divide-by-zero because, in each case, the
				// outcode bit being tested guarantees the denominator is non-zero
				if (outcodeOut & TOP) {           // point is above the clip window
					x = x0 + (x1 - x0) * (roi.bottom - y0) / (y1 - y0);
					y = roi.bottom;
				} else if (outcodeOut & BOTTOM) { // point is below the clip window
					x = x0 + (x1 - x0) * (roi.top - y0) / (y1 - y0);
					y = roi.top;
				} else if (outcodeOut & RIGHT) {  // point is to the right of clip window
					y = y0 + (y1 - y0) * (roi.right - x0) / (x1 - x0);
					x = roi.right;
				} else if (outcodeOut & LEFT) {   // point is to the left of clip window
					y = y0 + (y1 - y0) * (roi.left - x0) / (x1 - x0);
					x = roi.left;
				}

				// Now we move outside point to intersection point to clip
				// and get ready for next pass.
				if (outcodeOut == outcode0) {
					x0 = x;
					y0 = y;
					outcode0 = ComputeOutCode(x0, y0);
				} else {
					x1 = x;
					y1 = y;
					outcode1 = ComputeOutCode(x1, y1);
				}
			}
		}
	}


	int s_shape::GetLineWidthInUM(int lineWeight) {
		static int const widths[] = {
			   0,
			  50,
			  90,
			 130,
			 150,
			 180,
			 200,
			 250,
			 300,
			 350,
			 400,
			 500,
			 530,
			 600,
			 700,
			 800,
			 900,
			1000,
			1060,
			1200,
			1400,
			1580,
			2000,
			2110,
		};

		if ( (lineWeight >= 0) and (lineWeight < std::size(widths)) ) {
			return widths[lineWeight];
		}

		return 0;
	};

	void s_shape::Draw(ICanvas& canvas) const {
		canvas.PreDraw(*this);
	}
	bool s_shape::DrawROI(ICanvas& canvas, rect_t const& rectROI) const {
		rect_t rectBoundary;
		rectBoundary.SetRectEmptyForMinMax2d();
		UpdateBoundary(rectBoundary);
		if (!rectBoundary.IntersectRect(rectROI).IsRectHavingLength2d())
			return false;
		Draw(canvas);
		return true;
	}

	bool s_shape::LoadFromCADJson(json_t& _j) {
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
		if (iter == map.end()) {
			static auto const empty = L""s;
			return empty;
		}
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
		s_shape::Draw(canvas);

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
				s_arcXY arc = s_arcXY::GetFromBulge(pt0.Bulge(), pt0, pt1);
				(s_shape&)arc = *(s_shape*)this;

				arc.Draw(canvas);

				canvas.LineTo(pt1);
			}
		}
	}

	boost::ptr_deque<s_shape> s_polyline::Split() const {
		boost::ptr_deque<s_shape> shapes;

		auto nPt = pts.size();
		if (!bLoop)
			nPt--;
		for (int iPt = 0; iPt < nPt; iPt++) {
			auto iPt2 = (iPt+1) % pts.size();
			auto pt0 = pts[iPt];
			auto pt1 = pts[iPt2];
			if (pt0.Bulge() == 0.0) {
				auto rLine = std::make_unique<s_line>();
				(s_shape&)*rLine = (s_shape const&)*this;
				rLine->pt0 = pt0;
				rLine->pt1 = pt1;
				shapes.push_back(std::move(rLine));
			} else {
				s_arcXY arc = s_arcXY::GetFromBulge(pt0.Bulge(), pt0, pt1);
				(s_shape&)arc = *(s_shape*)this;
				shapes.push_back(arc.NewClone());
			}
		}
		return shapes;
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

					std::unique_ptr<s_shape> rShape = CreateShapeFromEntityName(strEntityName);
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

				std::unique_ptr<s_shape> rShape = CreateShapeFromEntityName(strEntityName);
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

	bool s_drawing::AddEntity(std::unique_ptr<s_shape> rShape, std::map<string_t, s_layer*> const& mapLayers, std::map<string_t, s_block*> const& mapBlocks, rect_t& rectB) {
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

				CCoordTrans3d ct;
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


};
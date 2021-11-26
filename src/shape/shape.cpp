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

	//// from openCV
	//bool clipLine(gtl::xSize2i size, gtl::xPoint2d& pt1, gtl::xPoint2d& pt2) {
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



	// Cohen-Sutherland clipping algorithm clips a line from
	// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
	// diagonal from (xmin, ymin) to (xmax, ymax).
	enum fOUT_CODE : uint8_t {
		INSIDE	= 0b0000,
		LEFT	= 0b0001,
		RIGHT	= 0b0010,
		BOTTOM	= 0b0100,
		TOP		= 0b1000,
	};
};

namespace gtl::shape {

	bool CohenSutherlandLineClip(gtl::xRect2d roi, gtl::xPoint2d& pt0, gtl::xPoint2d& pt1) {
		if ( (roi.Width() <= 0) or (roi.Height() <= 0) )
			return false;

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
			else if (x > roi.right)
				(uint8_t&)code |= fOUT_CODE::RIGHT;
			if (y < roi.top)
				(uint8_t&)code |= fOUT_CODE::BOTTOM;
			else if (y > roi.bottom)
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
				if (outcodeOut & fOUT_CODE::TOP) {           // point is above the clip window
					x = x0 + (x1 - x0) * (roi.bottom - y0) / (y1 - y0);
					y = roi.bottom;
				} else if (outcodeOut & fOUT_CODE::BOTTOM) { // point is below the clip window
					x = x0 + (x1 - x0) * (roi.top - y0) / (y1 - y0);
					y = roi.top;
				} else if (outcodeOut & fOUT_CODE::RIGHT) {  // point is to the right of clip window
					y = y0 + (y1 - y0) * (roi.right - x0) / (x1 - x0);
					x = roi.right;
				} else if (outcodeOut & fOUT_CODE::LEFT) {   // point is to the left of clip window
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

		return accept;
	}


	int xShape::GetLineWidthInUM(int lineWeight) {
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

	void xShape::Draw(ICanvas& canvas) const {
		canvas.PreDraw(*this);
	}
	bool xShape::DrawROI(ICanvas& canvas, rect_t const& rectROI) const {
		rect_t rectBoundary;
		rectBoundary.SetRectEmptyForMinMax2d();
		UpdateBoundary(rectBoundary);
		if (!rectBoundary.IntersectRect(rectROI).IsNormalized())
			return false;
		Draw(canvas);
		return true;
	}

	bool xShape::LoadFromCADJson(json_t& _j) {
		using namespace std::literals;
		gtl::bjson<json_t> j(_j);
		//m_color.cr = (int)j["color"sv];
		m_strLayer = j["layer"sv];

		m_lineWeight = j["lWeight"];
		m_strLineType = j["lineType"];

		//eLineType = j["lineType"sv];
		m_crIndex = j["color"].value_or(0);
		m_color.cr = -1;
		if (j["color24"].json().is_int64()) {
			m_crIndex = 0;
			m_color.cr = (int)j["color24"];
		} else {
			if ( (m_crIndex > 0) and (m_crIndex < colorTable_s.size()) )
				m_color = colorTable_s[m_crIndex];
		}
		m_bVisible = j["visible"].value_or(true);
		m_bTransparent = j["transparency"].value_or(0) != false;

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
			{ "INSERT"s,				[](){ return std::make_unique<xInsert>(); } },
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


	void xLayer::Sort_Loop() {
		if (m_shapes.size() <= 1) {
			return;
		}

		// Search EndPoint if any -> Move it to front
		auto const dThreshold = 1.e-3;
		size_t n{m_shapes.size()-1};
		for (size_t i{}; i < n; i++) {
			auto const& r = m_shapes[i].GetStartEndPoint();
			if (!r)
				continue;
			auto [pt0, pt1] = *r;
			bool bFound0{}, bFound1{};
			for (size_t j{}; j < m_shapes.size(); j++) {
				if (i == j)
					continue;
				if (auto const& r = m_shapes[j].GetStartEndPoint(); r) {
					if (!bFound0) {
						auto d0 = pt0.Distance(r->first);
						auto d1 = pt0.Distance(r->second);
						if (d0 <= dThreshold or d1 <= dThreshold) {
							bFound0 = true;
						}
					}
					if (!bFound1) {
						auto d0 = pt1.Distance(r->first);
						auto d1 = pt1.Distance(r->second);
						if (d0 <= dThreshold or d1 <= dThreshold) {
							bFound1 = true;
						}
					}
					if (bFound0 and bFound1)
						break;
				}
			}
			if (!bFound0 or !bFound1) {
				if (i)
					std::swap(m_shapes.base().at(0), m_shapes.base().at(i));
				if (bFound0 and !bFound1)
					m_shapes.front().Reverse();
				break;
			}
		}

		for (size_t i{}; i < n; i++) {
			auto const& r = m_shapes[i].GetStartEndPoint();
			if (!r)
				continue;
			auto pt = r->second;

			auto minDist = DBL_MAX;
			size_t jMin = (size_t)(-1);
			bool bReverse{};
			auto i1{i+1};
			for (size_t j{i1}; j < m_shapes.size(); j++) {
				if (auto const& r = m_shapes[j].GetStartEndPoint(); r) {
					auto d1 = pt.Distance(r->first);
					auto d2 = pt.Distance(r->second);
					auto dist = std::min(d1, d2);
					if (dist < minDist) {
						bReverse = d1 > d2;
						minDist = dist;
						jMin = j;
					}
				}
			}
			if (jMin < m_shapes.size()) {
				std::swap(m_shapes.base().at(i1), m_shapes.base().at(jMin));
				if (bReverse)
					m_shapes[i1].Reverse();
			}
		}
	}

	bool xLayer::IsLoop(double dMinGap) const {
		if (m_shapes.empty())
			return false;
		point_t pt0, pt;
		if (auto r = m_shapes.front().GetStartEndPoint(); r) {
			pt0 = r->first;
			pt = r->second;
		}
		else
			return false;

		for (size_t i{1}; i < m_shapes.size(); i++) {
			if (auto const& r = m_shapes[i].GetStartEndPoint(); r) {
				auto [ pt1, pt2 ] = *r;
				if (pt.Distance(pt1) > dMinGap)
					return false;
				pt = pt2;
			} else {
				continue;
			}
		}

		return pt0.Distance(pt) <= dMinGap;
	}

	void xPolyline::Draw(ICanvas& canvas) const {
		xShape::Draw(canvas);
		if (m_pts.empty())
			return;

		canvas.MoveTo(m_pts[0]);

		auto nPt = m_pts.size();
		if (!m_bLoop)
			nPt--;
		for (int iPt = 0; iPt < nPt; iPt++) {
			auto iPt2 = (iPt+1) % m_pts.size();
			auto pt0 = m_pts[iPt];
			auto pt1 = m_pts[iPt2];
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

		auto nPt = m_pts.size();
		if (!m_bLoop)
			nPt--;
		for (int iPt = 0; iPt < nPt; iPt++) {
			auto iPt2 = (iPt+1) % m_pts.size();
			auto pt0 = m_pts[iPt];
			auto pt1 = m_pts[iPt2];
			if (pt0.Bulge() == 0.0) {
				auto rLine = std::make_unique<xLine>();
				(xShape&)*rLine = (xShape const&)*this;
				rLine->m_pt0 = pt0;
				rLine->m_pt1 = pt1;
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
			m_vars.clear();
			for (auto& var : jVars) {
				auto key = var.key();
				auto value = var.value();
				if (value.is_string()) {
					auto& str = value.as_string();
					m_vars[std::string(key.data(), key.size())] = gtl::ToString<gtl::shape::char_t, char8_t>(std::u8string{(char8_t const*)str.data(), str.size()});
				} else if (value.is_int64()) {
					m_vars[std::string(key.data(), key.size())] = (int)value.as_int64();
				} else if (value.is_double()) {
					m_vars[std::string(key.data(), key.size())] = value.as_double();
				}
			}
		}

		// line types
		{
			m_line_types.clear();
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
				m_line_types.push_back(std::move(lt));
			}
		}

		// layers
		std::map<string_t, xLayer*> mapLayers;	// cache
		{
			m_layers.clear();
			//layers.push_back(std::make_unique<xLayer>(L"0"));

			auto jLayers = jTOP["layers"].json().as_array();
			for (auto& item : jLayers) {
				bjson<json_t> j(item);

				auto rLayer = std::make_unique<xLayer>();
				rLayer->LoadFromCADJson(item);

				if (auto iterLineType = std::find_if(m_line_types.begin(), m_line_types.end(), [&rLayer](auto const& lt) { return lt.name == rLayer->m_strLineType; });
					iterLineType != m_line_types.end()) {

					rLayer->pLineType = &(*iterLineType);
				}

				mapLayers[rLayer->m_name] = rLayer.get();
				m_layers.push_back(std::move(rLayer));
			}
		}

		// block
		boost::ptr_deque<xBlock> blocks;
		std::map<string_t, xBlock*> mapBlocks;
		{
			blocks.clear();
			auto jBlocks = jTOP["blocks"].json().as_array();
			for (auto& item : jBlocks) {
				bjson<json_t> j(item);
				auto rBlock = std::make_unique<xBlock>();

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


				if (auto iterLineType = std::find_if(m_line_types.begin(), m_line_types.end(), [&rBlock](auto const& lt) { return lt.name == rBlock->m_strLineType; });
					iterLineType != m_line_types.end()) {

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

					rBlock->m_shapes.push_back(std::move(rShape));
				}

				mapBlocks[rBlock->m_name] = rBlock.get();
				blocks.push_back(std::move(rBlock));
			}
		}


		// Entities
		{
			auto jEntities = jTOP["mainBlock"].json().as_array();
			m_rectBoundary.SetRectEmptyForMinMax2d();
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

				AddEntity(std::move(rShape), mapLayers, mapBlocks, m_rectBoundary);
			}
		}

		return true;
	}

	bool xDrawing::AddEntity(std::unique_ptr<xShape> rShape, std::map<string_t, xLayer*> const& mapLayers, std::map<string_t, xBlock*> const& mapBlocks, rect_t& rectB) {
		if (!rShape)
			return false;

		switch (rShape->GetShapeType()) {
		case eSHAPE::insert :
			// todo :
			if (xInsert* pInsert = dynamic_cast<xInsert*>(rShape.get()); pInsert) {
				auto iter = mapBlocks.find(pInsert->m_name);
				if (iter == mapBlocks.end()) {
					DEBUG_PRINT(L"No Block : {}\n", pInsert->m_name);
					return false;
				}
				auto* pBlock = iter->second;
				if (!pBlock) {
					DEBUG_PRINT(L"Internal ERROR (Block Name : {})\n", pInsert->m_name);
					return false;
				}

				xCoordTrans3d ct;
				// todo : 순서 확인 (scale->rotate ? or rotate->scale ?)
				if (pInsert->m_xscale != 1.0) {
					ct.m_mat(0, 0) *= pInsert->m_xscale;
					ct.m_mat(0, 1) *= pInsert->m_xscale;
					ct.m_mat(0, 2) *= pInsert->m_xscale;
				}
				if (pInsert->m_yscale != 1.0) {
					ct.m_mat(1, 0) *= pInsert->m_yscale;
					ct.m_mat(1, 1) *= pInsert->m_yscale;
					ct.m_mat(1, 2) *= pInsert->m_yscale;
				}
				if (pInsert->m_zscale != 1.0) {
					ct.m_mat(2, 0) *= pInsert->m_zscale;
					ct.m_mat(2, 1) *= pInsert->m_zscale;
					ct.m_mat(2, 2) *= pInsert->m_zscale;
				}

				if (pInsert->m_angle != 0.0_rad) {
					ct.m_mat = ct.GetRotatingMatrixXY(pInsert->m_angle) * ct.m_mat;
				}

				ct.m_origin = pBlock->m_pt;

				for (int y = 0; y < pInsert->m_nRow; y++) {
					for (int x = 0; x < pInsert->m_nCol; x++) {

						std::unique_ptr<xBlock> rBlockNew { dynamic_cast<xBlock*>(pBlock->NewClone().release()) };
						if (!rBlockNew)
							continue;

						ct.m_offset.x = x*pInsert->m_spacingCol + pInsert->m_pt.x;
						ct.m_offset.y = y*pInsert->m_spacingRow + pInsert->m_pt.y;
						bool const bIsRightHanded = ct.IsRightHanded();

						for (auto const& rShape : rBlockNew->m_shapes) {
							auto rShapeNew = rShape.NewClone();
							rShapeNew->Transform(ct, bIsRightHanded);
							// color
							if (rShapeNew->m_crIndex == 0) {
								rShapeNew->m_crIndex = pBlock->m_crIndex;
							}
							// line Width
							if (rShapeNew->m_lineWeight == (int)eLINE_WIDTH::ByBlock) {
								rShapeNew->m_lineWeight = pBlock->m_lineWeight;
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
			if (auto iter = mapLayers.find(rShape->m_strLayer); iter != mapLayers.end()) {
				auto* pLayer = iter->second;
				if (!pLayer) {
					DEBUG_PRINT(L"Internal Error : Layer {}\n", rShape->m_strLayer);
					return false;
				}

				// color
				if (rShape->m_color.cr == -1) {
					int m_crIndex = rShape->m_crIndex;
					if (m_crIndex == 256) {
						m_crIndex = pLayer->m_crIndex;
						rShape->m_color = pLayer->m_color;
					}
					if (rShape->m_color.cr == -1) {
						if ( (m_crIndex > 0) and (m_crIndex < colorTable_s.size()) )
							rShape->m_color = colorTable_s[m_crIndex];
					}
				}

				// line Width
				if (rShape->m_lineWeight == (int)eLINE_WIDTH::ByLayer) {
					rShape->m_lineWeight = pLayer->m_lineWeight;
				}

				rShape->UpdateBoundary(rectB);
				pLayer->m_shapes.push_back(std::move(rShape));

			} else {
				DEBUG_PRINT(L"No Layer : {}\n", rShape->m_strLayer);
				return false;
			}

			break;
		}

		return true;
	}


};

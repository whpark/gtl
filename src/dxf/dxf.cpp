//////////////////////////////////////////////////////////////////////
//
// dxf.cpp : gtl.dxf
//
// PWH
// 2026-09-02 biscuit.dxf -> gtl.dxf
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "gtl/dxf/dxf.h"

namespace gtl::dxf {

using namespace std::literals;
using namespace gtl::literals;

using layer_map_t = std::map<gtl::shape::string_t, gtl::shape::xLayer*>;
using block_map_t = std::map<gtl::shape::string_t, gtl::shape::xBlock*>;

//-----------------------------------------------------------------------------------------------------------------------------
/// @brief the codepage of the narrow strings inside a dxf file.
/// @param strACADVER $ACADVER		ex) "AC1021"
/// @param strCodepage $DWGCODEPAGE	ex) "ANSI_949"
/// @return R2007(AC1021) and later are always UTF-8. otherwise, $DWGCODEPAGE. (eCODEPAGE::DEFAULT : system MBCS)
gtl::eCODEPAGE CodepageOfDXF(std::string_view strACADVER, std::string_view strCodepage) {
	if (strACADVER.starts_with("AC") and strACADVER.size() >= 6 and strACADVER.substr(2) >= "1021"sv)
		return gtl::eCODEPAGE::UTF8;

	auto sv = gtl::TrimView(strCodepage);
	if (sv.empty())
		return gtl::eCODEPAGE::DEFAULT;
	std::string str;
	str.reserve(sv.size());
	for (auto c : sv)
		str += (char)std::tolower((unsigned char)c);
	if (str == "utf8" or str == "utf-8")
		return gtl::eCODEPAGE::UTF8;
	// "ANSI_949", "ANSI_1252", "DOS437", ... -> the trailing number is the codepage.
	if (auto pos = str.find_first_of("0123456789"); pos != str.npos) {
		int codepage{};
		std::from_chars(str.data() + pos, str.data() + str.size(), codepage);
		if (codepage > 0)
			return (gtl::eCODEPAGE)codepage;
	}
	return gtl::eCODEPAGE::DEFAULT;
}

//-----------------------------------------------------------------------------------------------------------------------------
/// @brief conversion context. (layer cache + string codepage)
struct sShapeConvertContext {
	layer_map_t layers;
	gtl::eCODEPAGE eCodepage{gtl::eCODEPAGE::UTF8};

	/// @brief dxf (narrow) string -> gtl::shape (wide) string. NEVER throws.
	gtl::shape::string_t Str(string_t const& str) const {
		if (str.empty())
			return {};
		try {
			return gtl::ToString<gtl::shape::char_t, char>(std::string_view(str), {.from = eCodepage});
		}
		catch (...) {}
	 // the declared codepage may be wrong (or the text may be mixed). try the system MBCS codepage.
		if (eCodepage != gtl::eCODEPAGE::DEFAULT) {
			try {
				return gtl::ToString<gtl::shape::char_t, char>(std::string_view(str), {.from = gtl::eCODEPAGE::DEFAULT});
			}
			catch (...) {}
		}
		// last resort : byte -> char, 1:1.
		gtl::shape::string_t result;
		result.reserve(str.size());
		for (auto c : str)
			result += (gtl::shape::char_t)(unsigned char)c;
		return result;
	}
};

//-----------------------------------------------------------------------------------------------------------------------------
gtl::shape::point_t ShapePoint(point_t const& pt) {
	return gtl::shape::point_t{pt.x, pt.y, pt.z};
}
gtl::shape::polypoint_t PolyPoint(point_t const& pt, double bulge = {}) {
	return gtl::shape::polypoint_t{pt.x, pt.y, pt.z, bulge};
}

int ShapeLineWeight(int value) {
	if (value == -1)
		return (int)gtl::shape::xShape::eLINE_WIDTH::ByLayer;
	if (value == -2)
		return (int)gtl::shape::xShape::eLINE_WIDTH::ByBlock;
	if (value == -3)
		return (int)gtl::shape::xShape::eLINE_WIDTH::Default;

	constexpr std::array values{
		0, 5, 9, 13, 15, 18, 20, 25, 30, 35, 40, 50,
		53, 60, 70, 80, 90, 100, 106, 120, 140, 158, 200, 211
	};
	if (auto iter = std::ranges::find(values, value); iter != values.end())
		return (int)std::distance(values.begin(), iter);
	return (int)gtl::shape::xShape::eLINE_WIDTH::Default;
}

gtl::shape::color_t IndexedColor(int index, gtl::shape::color_t fallback = gtl::shape::colorTable_s[7]) {
	index = std::abs(index);
	if (index > 0 and index < (int)gtl::shape::colorTable_s.size())
		return gtl::shape::colorTable_s[(size_t)index];
	return fallback;
}

gtl::shape::color_t TrueColor(gtl::color_bgra_t color) {
	return ColorRGBA(color);
}

/// @brief gtl::shape::xDrawing::Layer() does NOT create a missing layer. (biscuit::shape::xDrawing does)
gtl::shape::xLayer& EnsureLayer(gtl::shape::xDrawing& drawing, layer_map_t& layers, gtl::shape::string_t name) {
	if (name.empty())
		name = L"0"s;
	if (auto iter = layers.find(name); iter != layers.end())
		return *iter->second;

	auto rLayer = std::make_unique<gtl::shape::xLayer>(name);
	auto* pLayer = rLayer.get();
	drawing.m_layers.push_back(std::move(rLayer));
	pLayer->m_name = name;
	pLayer->m_color = gtl::shape::colorTable_s[7];
	layers.emplace(name, pLayer);
	return *pLayer;
}

void ApplyCommon(gtl::shape::xShape& target, entities::xEntity const& source, sShapeConvertContext const& ctx) {
	auto layerName = ctx.Str(source.m_entity.layer());
	if (layerName.empty())
		layerName = L"0"s;
	target.m_strLayer = layerName;
	target.m_strLineType = ctx.Str(source.m_entity.lineTypeName());
	target.m_lineWeight = ShapeLineWeight(source.m_entity.lineWeight());
	target.m_bVisible = !source.m_entity.bHidden();
	target.m_bTransparent = source.m_entity.transparency() != 0;

	gtl::shape::color_t layerColor = gtl::shape::colorTable_s[7];
	if (auto iter = ctx.layers.find(layerName); iter != ctx.layers.end() and iter->second)
		layerColor = iter->second->m_color;

	auto trueColor = source.m_entity.color24();
	if (trueColor.cr != 0) {
		target.m_color = TrueColor(trueColor);
		return;
	}

	int color = (int)source.m_entity.color();
	if (color < 0)
		target.m_bVisible = false;
	target.m_color = (color == (int)entities::eCOLOR::byLayer or color == (int)entities::eCOLOR::byBlock)
		? layerColor
		: IndexedColor(color, layerColor);
}

template <typename TShape>
std::unique_ptr<TShape> Finish(std::unique_ptr<TShape> target, entities::xEntity const& source, sShapeConvertContext const& ctx) {
	ApplyCommon(*target, source, ctx);
	return target;
}

std::unique_ptr<gtl::shape::xShape> ConvertSimpleEntity(entities::xEntity const& entity, sShapeConvertContext const& ctx) {
	switch (entity.GetEntityType()) {
		case entities::eENTITY::_3dface: {
			auto const* source = dynamic_cast<entities::x3DFace const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<gtl::shape::xPolyline>();
			target->m_bLoop = true;
			target->m_pts = {PolyPoint(field.pt1()), PolyPoint(field.pt2()), PolyPoint(field.pt3()), PolyPoint(field.pt4())};
			return Finish(std::move(target), entity, ctx);
		}
		case entities::eENTITY::line: {
			auto const* source = dynamic_cast<entities::xLine const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<gtl::shape::xLine>();
			target->m_pt0 = ShapePoint(field.pt0());
			target->m_pt1 = ShapePoint(field.pt1());
			return Finish(std::move(target), entity, ctx);
		}
		case entities::eENTITY::circle: {
			auto const* source = dynamic_cast<entities::xCircle const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<gtl::shape::xCircle>();
			target->m_ptCenter = ShapePoint(field.pt());
			target->m_radius = field.radius();
			target->m_angle_length = field.extrusion().z < 0 ? -360._deg : 360._deg;
			return Finish(std::move(target), entity, ctx);
		}
		case entities::eENTITY::arc: {
			auto const* source = dynamic_cast<entities::xArc const*>(&entity);
			if (!source) return {};
			auto const& fields = source->Subclasses();
			auto target = std::make_unique<gtl::shape::xArc>();
			target->m_ptCenter = ShapePoint(fields.m_circle.pt());
			target->m_radius = fields.m_circle.radius();
			target->m_angle_start = deg_t{fields.m_field.startAngle()};
			auto length = fields.m_field.endAngle() - fields.m_field.startAngle();
			while (length < 0.) length += 360.;
			target->m_angle_length = deg_t{length};
			if (fields.m_field.extrusion().z < 0)
				target->Reverse();
			return Finish(std::move(target), entity, ctx);
		}
		case entities::eENTITY::ellipse: {
			auto const* source = dynamic_cast<entities::xEllipse const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<gtl::shape::xEllipse>();
			target->m_ptCenter = ShapePoint(field.pt());
			auto const major = ShapePoint(field.vcMajor());
			target->m_radius = std::hypot(major.x, major.y);
			target->m_radiusH = target->m_radius * field.ratio();
			target->m_angle_first_axis = rad_t{std::atan2(major.y, major.x)};
			target->m_angle_start = deg_t{field.angle0()};
			target->m_angle_length = deg_t{field.angle1() - field.angle0()};
			if (field.extrusion().z < 0)
				target->Reverse();
			return Finish(std::move(target), entity, ctx);
		}
		case entities::eENTITY::point: {
			auto const* source = dynamic_cast<entities::xPoint const*>(&entity);
			if (!source) return {};
			auto target = std::make_unique<gtl::shape::xDot>();
			target->m_pt = ShapePoint(source->Subclasses().m_field.pt());
			return Finish(std::move(target), entity, ctx);
		}
		case entities::eENTITY::lw_polyline: {
			auto const* source = dynamic_cast<entities::xLWPolyline const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<gtl::shape::xPolylineLW>();
			target->m_bLoop = ((int)field.ePolylineFlag() & 1) != 0;
			for (auto const& vertex : field.vertices()) {
				auto pt = ShapePoint(vertex.pt());
				if (pt.z == 0.) pt.z = field.elevation();
				target->m_pts.emplace_back(pt.x, pt.y, pt.z, vertex.bulge());
			}
			if (field.extrusion().z < 0)
				target->FlipX();
			return Finish(std::move(target), entity, ctx);
		}
		case entities::eENTITY::spline: {
			auto const* source = dynamic_cast<entities::xSpline const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<gtl::shape::xSpline>();
			target->m_flags = (int)field.flags();
			target->m_degree = field.degree();
			target->m_ptNormal = ShapePoint(field.vcNormal());
			target->m_vStart = ShapePoint(field.vcStartTangent());
			target->m_vEnd = ShapePoint(field.vcEndTangent());
			target->m_toleranceKnot = field.toleranceKnot();
			target->m_toleranceControlPoint = field.toleranceControlPoint();
			target->m_toleranceFitPoint = field.toleranceFit();
			for (auto const& knot : field.knots()) target->m_knots.push_back(knot.value);
			for (auto const& pt : field.controlPoints()) target->m_ptsControl.push_back(ShapePoint(pt.value));
			for (auto const& pt : field.fitPoints()) target->m_ptsFit.push_back(ShapePoint(pt.value));
			return Finish(std::move(target), entity, ctx);
		}
		case entities::eENTITY::text: {
			auto const* source = dynamic_cast<entities::xText const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<gtl::shape::xText>();
			target->m_pt0 = ShapePoint(field.ptAlign0());
			target->m_pt1 = ShapePoint(field.ptAlign1());
			target->m_text = ctx.Str(field.text());
			target->m_height = field.heightText();
			target->m_angle = deg_t{field.rotation()};
			target->m_widthScale = field.scaleX();
			target->m_oblique = deg_t{field.angleOblique()};
			target->m_textStyle = ctx.Str(field.textStyleName());
			target->m_textgen = (int)field.fFlags();
			target->m_alignHorz = (gtl::shape::xText::eALIGN_HORZ)(int)field.eHorzJustification();
			target->m_alignVert = (gtl::shape::xText::eALIGN_VERT)(int)field.vertical_alignment();
			return Finish(std::move(target), entity, ctx);
		}
		case entities::eENTITY::mtext: {
			auto const* source = dynamic_cast<entities::xMText const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<gtl::shape::xMText>();
			target->m_pt0 = ShapePoint(field.pt());
			target->m_pt1 = target->m_pt0;
			target->m_text = ctx.Str(field.text());
			target->m_height = field.height();
			target->m_angle = deg_t{field.rotation()};
			target->m_textStyle = ctx.Str(field.textStyleName());
			target->m_interlin = field.lineSpacingFactor();
			target->SetAttachPoint((gtl::shape::xMText::eATTACH)(int)field.eAttachmentPoint());
			return Finish(std::move(target), entity, ctx);
		}
		case entities::eENTITY::solid:
		case entities::eENTITY::trace: {
			auto const* source = entity.GetEntityType() == entities::eENTITY::solid
				? dynamic_cast<entities::xSolid const*>(&entity)
				: nullptr;
			auto const* trace = dynamic_cast<entities::xTrace const*>(&entity);
			auto const* field = source ? &source->Subclasses().m_field : trace ? &trace->Subclasses().m_field : nullptr;
			if (!field) return {};
			auto target = std::make_unique<gtl::shape::xPolyline>();
			target->m_bLoop = true;
			target->m_pts = {PolyPoint(field->pt1()), PolyPoint(field->pt2()), PolyPoint(field->pt3()), PolyPoint(field->pt4())};
			return Finish(std::move(target), entity, ctx);
		}
		default:
			return {};
	}
}

std::unique_ptr<gtl::shape::xPolyline> ConvertPolyline(entities::xPolyline const& source, entities::entities_t const& entities, size_t& index, sShapeConvertContext const& ctx) {
	auto target = std::make_unique<gtl::shape::xPolyline>();
	auto const& field = source.Subclasses().m_field;
	target->m_bLoop = ((int)field.fFlags() & entities::fPOLYLINE__CLOSED) != 0;
	OutputDebugStringA(std::format("ConvertPolyline: Polyline {:#x} {} {}\n", (uint)field.fFlags(), index, target->m_bLoop ? "closed" : "open").c_str());
	if (index == 223) {
		OutputDebugStringA("ConvertPolyline: Polyline 223\n");
	}
	for (size_t i = index + 1; i < entities.size(); ++i) {
		auto const& item = *entities[i];
		if (auto const* vertex = dynamic_cast<entities::xVertex const*>(&item)) {
			auto const& value = vertex->Subclasses().m_field;
			auto pt = ShapePoint(value.pt());
			if (!field.b3d and pt.z == 0.) pt.z = field.elevation().z;
			target->m_pts.emplace_back(pt.x, pt.y, pt.z, value.bulge());
			index = i;
			OutputDebugStringA(std::format("ConvertPolyline: vertex {}: ({}, {}, {}) bulge={}\n", i, pt.x, pt.y, pt.z, value.bulge()).c_str());
			continue;
		}
		if (auto const* unknown = dynamic_cast<entities::xUnknown const*>(&item); unknown and unknown->m_name == "SEQEND"s)
			index = i;
		break;
	}
	//if (field.fFlags.value >= entities::fPOLYLINE__3D) {
	//	// 3d polyline
	//	return {};
	//}
	//else if (field.fFlags.value >= entities::fPOLYLINE__CURVE_FIT) {
	//	// curve fit polyline
	//	return {};
	//}
	//else if (field.fFlags.value >= entities::fPOLYLINE__SPLINE_FIT) {
	//	// spline fit polyline
	//	return {};
	//}
	//else {
	//	// 2d polyline
	//}
	ApplyCommon(*target, source, ctx);
	return target;
}

template <typename F>
void CloneBlock(entities::xInsert const& source, gtl::shape::xBlock const& block, F&& add) {
	auto const& field = source.Subclasses().m_field;
	gtl::xCoordTrans3d transform;
	auto const& scale = field.scale();
	if (scale.x != 1.) {
		transform.m_mat(0, 0) *= scale.x;
		transform.m_mat(0, 1) *= scale.x;
		transform.m_mat(0, 2) *= scale.x;
	}
	if (scale.y != 1.) {
		transform.m_mat(1, 0) *= scale.y;
		transform.m_mat(1, 1) *= scale.y;
		transform.m_mat(1, 2) *= scale.y;
	}
	if (scale.z != 1.) {
		transform.m_mat(2, 0) *= scale.z;
		transform.m_mat(2, 1) *= scale.z;
		transform.m_mat(2, 2) *= scale.z;
	}

	deg_t const angle{field.angleRotation()};
	if (angle != 0._deg)
		transform.m_mat = transform.GetRotatingMatrixXY(angle) * transform.m_mat;

	transform.m_origin = block.m_pt;
	auto const insertionPoint = ShapePoint(field.ptInsertionPoint());
	bool const isRightHanded = transform.IsRightHanded();

	transform.m_offset.z = insertionPoint.z;
	for (int row{}; row < field.rows(); ++row) {
		transform.m_offset.y = row * field.rowSpacing() + insertionPoint.y;
		for (int col{}; col < field.cols(); ++col) {
			transform.m_offset.x = col * field.colSpacing() + insertionPoint.x;
			for (auto const& sourceShape : block.m_shapes) {
				auto item = sourceShape.NewClone();		// gtl : NewClone(), biscuit : clone()
				if (!item)
					continue;
				item->Transform(transform, isRightHanded);
				if (item->m_lineWeight == (int)gtl::shape::xShape::eLINE_WIDTH::ByBlock)
					item->m_lineWeight = block.m_lineWeight;
				// todo: color 처리 해야 하는데...
				add(std::move(item));
			}
		}
	}
}

template <typename FAdd, typename FFindBlock>
void ConvertEntities(entities::entities_t const& entities, sShapeConvertContext const& ctx, FAdd&& add, FFindBlock&& findBlock) {
	for (size_t i{}; i < entities.size(); ++i) {
		auto const& entity = *entities[i];
		if (auto const* insert = dynamic_cast<entities::xInsert const*>(&entity)) {
			auto const& blockName = insert->Subclasses().m_field.blockName();
			if (auto const* block = findBlock(ctx.Str(blockName)); block)
				CloneBlock(*insert, *block, add);
			continue;
		}

		std::unique_ptr<gtl::shape::xShape> converted;
		if (auto const* polyline = dynamic_cast<entities::xPolyline const*>(&entity))
			converted = ConvertPolyline(*polyline, entities, i, ctx);
		else
			converted = ConvertSimpleEntity(entity, ctx);
		if (converted)
			add(std::move(converted));
	}
}

gtl::shape::xDrawing ConvertToShape(xSectionTables const& tables, xSectionBlocks const& blocks, xSectionEntities const& sectionEntities, gtl::eCODEPAGE eCodepage = gtl::eCODEPAGE::UTF8) {
	gtl::shape::xDrawing drawing;
	sShapeConvertContext ctx;
	ctx.eCodepage = eCodepage;
	auto& layers = ctx.layers;

	for (auto const& [header, record] : tables.m_layers.items) {
		auto name = ctx.Str(record.name());
		auto& layer = EnsureLayer(drawing, layers, name);
		layer.m_flags = (int)record.flags();
		layer.m_bUse = !record.bDoNotPlot();
		layer.m_strLineType = ctx.Str(record.linetype());
		layer.m_lineWeight = ShapeLineWeight(record.eLineWeight());
		auto trueColor = record.color24();
		layer.m_color = trueColor.cr != 0 ? TrueColor(trueColor) : IndexedColor(record.iColor());
	}
	EnsureLayer(drawing, layers, L"0"s);
	for (auto const& sourceBlock : blocks.m_blocks) {
		for (auto const& entity : sourceBlock.entities)
			EnsureLayer(drawing, layers, ctx.Str(entity->m_entity.layer()));
	}
	for (auto const& entity : sectionEntities.m_entities)
		EnsureLayer(drawing, layers, ctx.Str(entity->m_entity.layer()));

	for (auto const& [header, record] : tables.m_lineTypes.items) {
		auto lineType = std::make_unique<gtl::shape::line_type_t>();
		lineType->name = ctx.Str(record.name());
		lineType->flags = (int)record.fStandard();
		lineType->description = ctx.Str(record.linetypeDescription());
		for (auto const& item : record.details())
			lineType->path.push_back(item.lenDashDotSpace());
		drawing.m_line_types.push_back(std::move(lineType));
	}

	std::deque<std::unique_ptr<gtl::shape::xBlock>> shapeBlocks;
	block_map_t blockMap;
	std::map<gtl::shape::string_t, xBlock const*> sourceBlockMap;
	for (auto const& sourceBlock : blocks.m_blocks) {
		shapeBlocks.push_back(std::make_unique<gtl::shape::xBlock>());
		auto& targetBlock = *shapeBlocks.back();
		targetBlock.m_name = ctx.Str(sourceBlock.header.block.name());
		targetBlock.m_layer = ctx.Str(sourceBlock.header.entity.layer());
		targetBlock.m_pt = ShapePoint(sourceBlock.header.block.ptBase());
		blockMap[targetBlock.m_name] = &targetBlock;
		sourceBlockMap[targetBlock.m_name] = &sourceBlock;
	}

	enum class eBLOCK_STATE { none, building, complete };
	std::map<gtl::shape::string_t, eBLOCK_STATE> blockStates;
	std::function<gtl::shape::xBlock* (gtl::shape::string_t const&)> buildBlock;
	buildBlock = [&](gtl::shape::string_t const& name) -> gtl::shape::xBlock* {
		auto targetIter = blockMap.find(name);
		auto sourceIter = sourceBlockMap.find(name);
		if (targetIter == blockMap.end() or sourceIter == sourceBlockMap.end())
			return nullptr;

		auto& state = blockStates[name];
		if (state == eBLOCK_STATE::complete)
			return targetIter->second;
		if (state == eBLOCK_STATE::building)
			return nullptr;

		state = eBLOCK_STATE::building;
		auto* targetBlock = targetIter->second;
		ConvertEntities(sourceIter->second->entities, ctx,
			[&](std::unique_ptr<gtl::shape::xShape> item) {
			targetBlock->m_shapes.push_back(std::move(item));
		},
			[&](gtl::shape::string_t const& referencedName) -> gtl::shape::xBlock* {
			return buildBlock(referencedName);
		});
		state = eBLOCK_STATE::complete;
		return targetBlock;
	};

	for (auto const& [name, block] : blockMap)
		buildBlock(name);

	drawing.m_rectBoundary.SetRectEmptyForMinMax2d();
	ConvertEntities(sectionEntities.m_entities, ctx,
		[&](std::unique_ptr<gtl::shape::xShape> item) {
		drawing.AddEntity(std::move(item), layers, blockMap, drawing.m_rectBoundary);
	},
		[&](gtl::shape::string_t const& name) -> gtl::shape::xBlock* {
		return buildBlock(name);
	});

	return drawing;
}



string_t GetHeaderString(xSectionHead const& header, string_t const& name) {
	auto const& vars = header.m_mapVariables;			// const -> TContainerMap::find() does NOT insert
	if (auto iter = vars.find(name); iter != vars.end()) {
		for (auto const& group : iter->second) {
			if (auto v = group.GetValue<string_t>())
				return *v;
		}
	}
	return {};
}

gtl::shape::xDrawing ToShape(xDXF const& dxf) {
	auto const& header = static_cast<xSectionHead const&>(dxf);
	auto const eCodepage = CodepageOfDXF(GetHeaderString(header, "$ACADVER"), GetHeaderString(header, "$DWGCODEPAGE"));
	return ConvertToShape(
		static_cast<xSectionTables const&>(dxf),
		static_cast<xSectionBlocks const&>(dxf),
		static_cast<xSectionEntities const&>(dxf),
		eCodepage);
}

std::optional<gtl::shape::xDrawing> ReadDXFShape(std::filesystem::path const& path) {
	xDXF dxf;
	if (!dxf.ReadDXF(path))
		return std::nullopt;
	return ToShape(dxf);
}

bool xDXF::ReadDXF(std::filesystem::path const& path) {
	if (auto g = ReadGroups(path); g)
		m_groups = std::move(*g);
	else
		return false;

	// TopMost. Read SECTIONs
	auto& groups = m_groups;

	// NOTE : biscuit uses TContainerMap here. gtl::TContainerMap::find() inserts a new item
	//        when the key is missing, so a plain std::map is used instead.
	std::map<std::string, std::function<bool(this_t& self, group_iter_t& iter)>> mapReader;
	mapReader["HEADER"]			= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionHead&>(self), iter); };
	mapReader["CLASSES"]		= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionClasses&>(self), iter); };
	mapReader["TABLES"]			= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionTables&>(self), iter); };
	mapReader["BLOCKS"]			= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionBlocks&>(self), iter); };
	mapReader["ENTITIES"]		= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionEntities&>(self), iter); };
	mapReader["OBJECTS"]		= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionObjects&>(self), iter); };
	mapReader["THUMBNAILIMAGE"] = [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionThumbnailImage&>(self), iter); };
	mapReader["ACDSDATA"]		= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionACDSDATA&>(self), iter); };

	for (group_iter_t iter{groups}; iter; ) {
		// Read Section Mark
		{
			static sGroup const groupEOF{0, "EOF"s};
			static sGroup const groupSectionStart{0, "SECTION"s};
			auto const& r = *iter;
			if (r == groupEOF)
				break;
			if (r != groupSectionStart)
				return false;
			if (!++iter)
				break;
		}

		// Read Section Content
		auto const& group = *iter;
		auto section = group.GetValue<string_t>();
		if (group.eCode != 2 or !section) {
			auto pos = iter - groups.cbegin()+1;
			fmt::println("ReadDXF: invalid section name, pos: {}(rec:{}), group: {}", 2*pos, pos, *iter);
			return false;
		}
		if (auto p = mapReader.find(*section); p != mapReader.end() and p->second) {
			auto& reader = p->second;
			auto pos = iter - groups.cbegin()+1;
			if (!++iter or !reader(*this, iter)) {
				auto pos = iter - groups.cbegin()+1;
				fmt::println("ReadDXF Section - Error, pos: {}(rec:{}), group: {}", 2*pos, pos, iter ? *iter : sGroup{});
				return false;
			}
			mapReader.erase(p);
		}
		else {
			auto pos = iter - groups.cbegin()+1;
			fmt::println("ReadDXF Section - Unknown section: {}, pos: {}(rec:{})", *section, 2*pos, pos);
			xSectionUnknown section;
			if (!++iter or !ReadSection(section, iter)) {
				auto pos = iter - groups.cbegin()+1;
				fmt::println("ReadDXF Section - Error, pos: {}(rec:{}), group: {}", 2*pos, pos, iter ? *iter : sGroup{});
				return false;
			}
		}
	}
	//return mapReader.size() <= 1;
	return mapReader.find("ENTITIES") == mapReader.end();
}


}	// namespace gtl::dxf

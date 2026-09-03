#pragma once

#include "gtl/dxf/sections.h"

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::dxf::detail {

	using layer_map_t = std::map<shape::string_t, shape::xLayer*>;
	using block_map_t = std::map<shape::string_t, shape::xBlock*>;

	inline shape::string_t ShapeString(string_view_t value) {
		return ToStringW(value);
	}

	inline shape::point_t const& ShapePoint(point_t const& pt) {
		return reinterpret_cast<shape::point_t const&>(pt);
		//return {pt.x, pt.y, pt.z};
	}

	inline int ShapeLineWeight(int value) {
		if (value == -1)
			return (int)shape::xShape::eLINE_WIDTH::ByLayer;
		if (value == -2)
			return (int)shape::xShape::eLINE_WIDTH::ByBlock;
		if (value == -3)
			return (int)shape::xShape::eLINE_WIDTH::Default;

		constexpr std::array values{
			0, 5, 9, 13, 15, 18, 20, 25, 30, 35, 40, 50,
			53, 60, 70, 80, 90, 100, 106, 120, 140, 158, 200, 211
		};
		if (auto iter = std::ranges::find(values, value); iter != values.end())
			return (int)std::distance(values.begin(), iter);
		return (int)shape::xShape::eLINE_WIDTH::Default;
	}

	inline shape::color_t IndexedColor(int index, shape::color_t fallback = shape::colorTable_s[7]) {
		index = std::abs(index);
		if (index > 0 and index < (int)shape::colorTable_s.size())
			return shape::colorTable_s[(size_t)index];
		return fallback;
	}

	inline shape::color_t TrueColor(color_bgra_t color) {
		return ColorRGBA(color);
	}

	inline shape::xLayer& EnsureLayer(shape::xDrawing& drawing, layer_map_t& layers, string_t name) {
		if (name.empty())
			name = "0"s;
		auto const shapeName = ShapeString(name);
		if (auto iter = layers.find(shapeName); iter != layers.end())
			return *iter->second;

		drawing.m_layers.push_back(new shape::xLayer(shapeName));
		auto& layer = drawing.m_layers.back();
		layer.m_color = shape::colorTable_s[7];
		layers.emplace(shapeName, &layer);
		return layer;
	}

	inline void ApplyCommon(shape::xShape& target, entities::xEntity const& source, layer_map_t const& layers) {
		auto layerName = source.m_entity.layer();
		if (layerName.empty())
			layerName = "0"s;
		auto const shapeLayerName = ShapeString(layerName);
		target.m_strLayer = shapeLayerName;
		target.m_strLineType = ShapeString(source.m_entity.lineTypeName());
		target.m_lineWeight = ShapeLineWeight(source.m_entity.lineWeight());
		target.m_bVisible = !source.m_entity.bHidden();
		target.m_bTransparent = source.m_entity.transparency() != 0;

		shape::color_t layerColor = shape::colorTable_s[7];
		if (auto iter = layers.find(shapeLayerName); iter != layers.end() and iter->second)
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
	std::unique_ptr<TShape> Finish(std::unique_ptr<TShape> target, entities::xEntity const& source, layer_map_t const& layers) {
		ApplyCommon(*target, source, layers);
		return target;
	}

	inline std::unique_ptr<shape::xShape> ConvertSimpleEntity(entities::xEntity const& entity, layer_map_t const& layers) {
		switch (entity.GetEntityType()) {
		case entities::eENTITY::_3dface: {
			auto const* source = dynamic_cast<entities::x3DFace const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<shape::xPolyline>();
			target->m_bLoop = true;
			target->m_pts = {ShapePoint(field.pt1()), ShapePoint(field.pt2()), ShapePoint(field.pt3()), ShapePoint(field.pt4())};
			return Finish(std::move(target), entity, layers);
		}
		case entities::eENTITY::line: {
			auto const* source = dynamic_cast<entities::xLine const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<shape::xLine>();
			target->m_pt0 = ShapePoint(field.pt0());
			target->m_pt1 = ShapePoint(field.pt1());
			return Finish(std::move(target), entity, layers);
		}
		case entities::eENTITY::circle: {
			auto const* source = dynamic_cast<entities::xCircle const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<shape::xCircle>();
			target->m_ptCenter = ShapePoint(field.pt());
			target->m_radius = field.radius();
			target->m_angle_length = field.extrusion().z < 0 ? -360._deg : 360._deg;
			return Finish(std::move(target), entity, layers);
		}
		case entities::eENTITY::arc: {
			auto const* source = dynamic_cast<entities::xArc const*>(&entity);
			if (!source) return {};
			auto const& fields = source->Subclasses();
			auto target = std::make_unique<shape::xArc>();
			target->m_ptCenter = ShapePoint(fields.m_circle.pt());
			target->m_radius = fields.m_circle.radius();
			target->m_angle_start = deg_t{fields.m_field.startAngle()};
			auto length = fields.m_field.endAngle() - fields.m_field.startAngle();
			while (length < 0.) length += 360.;
			target->m_angle_length = deg_t{length};
			if (fields.m_field.extrusion().z < 0)
				target->Reverse();
			return Finish(std::move(target), entity, layers);
		}
		case entities::eENTITY::ellipse: {
			auto const* source = dynamic_cast<entities::xEllipse const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<shape::xEllipse>();
			target->m_ptCenter = ShapePoint(field.pt());
			auto const major = ShapePoint(field.vcMajor());
			target->m_radius = std::hypot(major.x, major.y);
			target->m_radiusH = target->m_radius * field.ratio();
			target->m_angle_first_axis = rad_t{std::atan2(major.y, major.x)};
			target->m_angle_start = deg_t{field.angle0()};
			target->m_angle_length = deg_t{field.angle1() - field.angle0()};
			if (field.extrusion().z < 0)
				target->Reverse();
			return Finish(std::move(target), entity, layers);
		}
		case entities::eENTITY::point: {
			auto const* source = dynamic_cast<entities::xPoint const*>(&entity);
			if (!source) return {};
			auto target = std::make_unique<shape::xDot>();
			target->m_pt = ShapePoint(source->Subclasses().m_field.pt());
			return Finish(std::move(target), entity, layers);
		}
		case entities::eENTITY::lw_polyline: {
			auto const* source = dynamic_cast<entities::xLWPolyline const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<shape::xPolylineLW>();
			target->m_bLoop = ((int)field.ePolylineFlag() & 1) != 0;
			for (auto const& vertex : field.vertices()) {
				auto pt = ShapePoint(vertex.pt());
				if (pt.z == 0.) pt.z = field.elevation();
				target->m_pts.emplace_back(pt.x, pt.y, pt.z, vertex.bulge());
			}
			if (field.extrusion().z < 0)
				target->FlipX();
			return Finish(std::move(target), entity, layers);
		}
		case entities::eENTITY::spline: {
			auto const* source = dynamic_cast<entities::xSpline const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<shape::xSpline>();
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
			return Finish(std::move(target), entity, layers);
		}
		case entities::eENTITY::text: {
			auto const* source = dynamic_cast<entities::xText const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<shape::xText>();
			target->m_pt0 = ShapePoint(field.ptAlign0());
			target->m_pt1 = ShapePoint(field.ptAlign1());
			target->m_text = ShapeString(field.text());
			target->m_height = field.heightText();
			target->m_angle = deg_t{field.rotation()};
			target->m_widthScale = field.scaleX();
			target->m_oblique = deg_t{field.angleOblique()};
			target->m_textStyle = ShapeString(field.textStyleName());
			target->m_textgen = (int)field.fFlags();
			target->m_alignHorz = (shape::xText::eALIGN_HORZ)(int)field.eHorzJustification();
			target->m_alignVert = (shape::xText::eALIGN_VERT)(int)field.vertical_alignment();
			return Finish(std::move(target), entity, layers);
		}
		case entities::eENTITY::mtext: {
			auto const* source = dynamic_cast<entities::xMText const*>(&entity);
			if (!source) return {};
			auto const& field = source->Subclasses().m_field;
			auto target = std::make_unique<shape::xMText>();
			target->m_pt0 = ShapePoint(field.pt());
			target->m_pt1 = target->m_pt0;
			target->m_text = ShapeString(field.text());
			target->m_height = field.height();
			target->m_angle = deg_t{field.rotation()};
			target->m_textStyle = ShapeString(field.textStyleName());
			target->m_interlin = field.lineSpacingFactor();
			target->SetAttachPoint((shape::xMText::eATTACH)(int)field.eAttachmentPoint());
			return Finish(std::move(target), entity, layers);
		}
		case entities::eENTITY::solid:
		case entities::eENTITY::trace: {
			auto const* source = entity.GetEntityType() == entities::eENTITY::solid
				? dynamic_cast<entities::xSolid const*>(&entity)
				: nullptr;
			auto const* trace = dynamic_cast<entities::xTrace const*>(&entity);
			auto const* field = source ? &source->Subclasses().m_field : trace ? &trace->Subclasses().m_field : nullptr;
			if (!field) return {};
			auto target = std::make_unique<shape::xPolyline>();
			target->m_bLoop = true;
			target->m_pts = {ShapePoint(field->pt1()), ShapePoint(field->pt2()), ShapePoint(field->pt3()), ShapePoint(field->pt4())};
			return Finish(std::move(target), entity, layers);
		}
		default:
			return {};
		}
	}

	inline std::unique_ptr<shape::xPolyline> ConvertPolyline(entities::xPolyline const& source, entities::entities_t const& entities, size_t& index, layer_map_t const& layers) {
		auto target = std::make_unique<shape::xPolyline>();
		auto const& field = source.Subclasses().m_field;
		target->m_bLoop = ((int)field.fFlags() & entities::fPOLYLINE__CLOSED) != 0;
		for (size_t i = index + 1; i < entities.size(); ++i) {
			auto const& item = *entities[i];
			if (auto const* vertex = dynamic_cast<entities::xVertex const*>(&item)) {
				auto const& value = vertex->Subclasses().m_field;
				auto pt = ShapePoint(value.pt());
				if (!field.b3d and pt.z == 0.) pt.z = field.elevation().z;
				target->m_pts.emplace_back(pt.x, pt.y, pt.z, value.bulge());
				index = i;
				continue;
			}
			if (auto const* unknown = dynamic_cast<entities::xUnknown const*>(&item); unknown and unknown->m_name == "SEQEND"s)
				index = i;
			break;
		}
		ApplyCommon(*target, source, layers);
		return target;
	}

	template <typename F>
	void CloneBlock(entities::xInsert const& source, shape::xBlock const& block, F&& add) {
		auto const& field = source.Subclasses().m_field;
		xCoordTrans3d transform;
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
				for (auto const& shape : block.m_shapes) {
					auto item = shape.NewClone();
					if (!item)
						continue;
					item->Transform(transform, isRightHanded);
					if (item->m_lineWeight == (int)shape::xShape::eLINE_WIDTH::ByBlock)
						item->m_lineWeight = block.m_lineWeight;
					// todo: color 처리 해야 하는데...
					add(std::move(item));
				}
			}
		}
	}

	template <typename FAdd, typename FFindBlock>
	void ConvertEntities(entities::entities_t const& entities, layer_map_t const& layers, FAdd&& add, FFindBlock&& findBlock) {
		for (size_t i{}; i < entities.size(); ++i) {
			auto const& entity = *entities[i];
			if (auto const* insert = dynamic_cast<entities::xInsert const*>(&entity)) {
				auto const& blockName = insert->Subclasses().m_field.blockName();
				if (auto const* block = findBlock(blockName); block)
					CloneBlock(*insert, *block, add);
				continue;
			}

			std::unique_ptr<shape::xShape> converted;
			if (auto const* polyline = dynamic_cast<entities::xPolyline const*>(&entity))
				converted = ConvertPolyline(*polyline, entities, i, layers);
			else
				converted = ConvertSimpleEntity(entity, layers);
			if (converted)
				add(std::move(converted));
		}
	}

	inline shape::xDrawing ConvertToShape(xSectionTables const& tables, xSectionBlocks const& blocks, xSectionEntities const& sectionEntities) {
		shape::xDrawing drawing;
		layer_map_t layers;

		for (auto const& [header, record] : tables.m_layers.items) {
			auto name = record.name();
			auto& layer = EnsureLayer(drawing, layers, name);
			layer.m_flags = (int)record.flags();
			layer.m_bUse = !record.bDoNotPlot();
			layer.m_strLineType = ShapeString(record.linetype());
			layer.m_lineWeight = ShapeLineWeight(record.eLineWeight());
			auto trueColor = record.color24();
			layer.m_color = trueColor.cr != 0 ? TrueColor(trueColor) : IndexedColor(record.iColor());
		}
		EnsureLayer(drawing, layers, "0"s);
		for (auto const& sourceBlock : blocks.m_blocks) {
			for (auto const& entity : sourceBlock.entities)
				EnsureLayer(drawing, layers, entity->m_entity.layer());
		}
		for (auto const& entity : sectionEntities.m_entities)
			EnsureLayer(drawing, layers, entity->m_entity.layer());

		for (auto const& [header, record] : tables.m_lineTypes.items) {
			auto lineType = std::make_unique<shape::line_type_t>();
			lineType->name = ShapeString(record.name());
			lineType->flags = (int)record.fStandard();
			lineType->description = ShapeString(record.linetypeDescription());
			for (auto const& item : record.details())
				lineType->path.push_back(item.lenDashDotSpace());
			drawing.m_line_types.push_back(lineType.release());
		}

		std::deque<shape::xBlock> shapeBlocks;
		block_map_t blockMap;
		std::map<string_t, xBlock const*> sourceBlockMap;
		for (auto const& sourceBlock : blocks.m_blocks) {
			shapeBlocks.emplace_back();
			auto& targetBlock = shapeBlocks.back();
			auto const sourceName = sourceBlock.header.block.name();
			targetBlock.m_name = ShapeString(sourceName);
			targetBlock.m_layer = ShapeString(sourceBlock.header.entity.layer());
			targetBlock.m_pt = ShapePoint(sourceBlock.header.block.ptBase());
			blockMap[targetBlock.m_name] = &targetBlock;
			sourceBlockMap[sourceName] = &sourceBlock;
		}

		enum class eBLOCK_STATE { none, building, complete };
		std::map<string_t, eBLOCK_STATE> blockStates;
		std::function<shape::xBlock*(string_t const&)> buildBlock;
		buildBlock = [&](string_t const& name) -> shape::xBlock* {
			auto targetIter = blockMap.find(ShapeString(name));
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
			ConvertEntities(sourceIter->second->entities, layers,
				[&](std::unique_ptr<shape::xShape> item) {
					targetBlock->m_shapes.push_back(std::move(item));
				},
				[&](string_t const& referencedName) -> shape::xBlock* {
					return buildBlock(referencedName);
				});
			state = eBLOCK_STATE::complete;
			return targetBlock;
		};

		for (auto const& [name, block] : sourceBlockMap)
			buildBlock(name);

		drawing.m_rectBoundary.SetRectEmptyForMinMax2d();
		ConvertEntities(sectionEntities.m_entities, layers,
			[&](std::unique_ptr<shape::xShape> item) {
				drawing.AddEntity(std::move(item), layers, blockMap, drawing.m_rectBoundary);
			},
			[&](string_t const& name) -> shape::xBlock* {
				return buildBlock(name);
			});

		return drawing;
	}

}

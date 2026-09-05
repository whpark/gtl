#pragma once

#include "aliases.h"
#include <variant>
#include <vector>

namespace gtl::dwg::entities {

	struct sLine { point_t start, end; };
	struct sCircle { point_t center; double radius{}; };
	struct sArc { point_t center; double radius{}, startAngle{}, endAngle{}; }; // radians
	struct sPoint { point_t position; };
	struct sPolyline {
		std::vector<point_t> points;
		std::vector<double> bulges;
		bool closed{};
		double constantWidth{};
		std::vector<std::pair<double, double>> widths;
		bool is3d{};
		std::uint16_t flags{}, curveType{};
		double elevation{}, defaultStartWidth{}, defaultEndWidth{};
		handle_t firstVertex{}, lastVertex{}, sequenceEnd{};
	};
	struct sVertex {
		point_t position;
		std::uint8_t flags{};
		double startWidth{}, endWidth{}, bulge{}, tangent{};
	};
	struct sInsert {
		point_t position, scale{1., 1., 1.};
		double rotation{}; // radians
		handle_t block{}, firstAttribute{}, lastAttribute{}, sequenceEnd{};
		bool hasAttributes{};
		std::uint16_t columns{1}, rows{1};
		double columnSpacing{}, rowSpacing{};
	};
	// monostate retains common metadata for structural and unsupported entities.
	using geometry_t = std::variant<std::monostate, sLine, sCircle, sArc, sPoint, sPolyline, sVertex, sInsert>;
	struct sEntity {
		handle_t handle{}, owner{}, layer{}, previous{}, next{};
		std::uint16_t type{};
		std::uint8_t mode{}; // 0: owner reference, 1: paper space, 2: model space
		std::int16_t color{256};
		std::uint8_t lineWeight{29}; // DWG enumeration, 29 = ByLayer
		bool invisible{};
		double lineTypeScale{1.}, thickness{};
		point_t extrusion{0., 0., 1.};
		geometry_t geometry;
	};

}

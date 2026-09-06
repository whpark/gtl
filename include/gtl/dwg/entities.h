#pragma once

#include "aliases.h"
#include <array>
#include <variant>
#include <optional>
#include <vector>

namespace gtl::dwg::entities {

	struct sLine { point_t start, end; };
	struct sCircle { point_t center; double radius{}; };
	struct sArc { point_t center; double radius{}, startAngle{}, endAngle{}; }; // radians
	struct sEllipse { point_t center, majorAxis; double ratio{}, startAngle{}, endAngle{}; }; // WCS, radians
	struct sSpline {
		std::uint32_t degree{}, scenario{}, flags{}, knotParameter{};
		bool rational{}, closed{}, periodic{};
		double knotTolerance{}, controlTolerance{}, fitTolerance{};
		point_t startTangent, endTangent;
		std::vector<double> knots, weights;
		std::vector<point_t> controlPoints, fitPoints; // WCS
	};
	struct sPoint { point_t position; };
	struct sFace3D { std::array<point_t,4> corners; std::uint16_t invisibleEdges{}; }; // WCS; bits 0..3 hide edges.
	struct sRay { point_t origin, direction; bool bothWays{}; }; // RAY or XLINE, unbounded WCS geometry.
	struct sShape { point_t insertion; double scale{}, rotation{}, widthFactor{}, oblique{}; std::int16_t number{}; handle_t style{}; };
	struct sFaceRecord { std::array<std::int16_t,4> indices{}; }; // Signed, one-based PFACE indices; negative hides the edge.
	struct sSolid { std::array<point_t,4> corners; }; // SOLID/TRACE corners in OCS, perimeter order 0,1,3,2.
	struct sMText {
		point_t insertion, normal, xAxis;
		string_t text;
		double width{}, height{}, textHeight{}, extentHeight{}, extentWidth{}, lineSpacing{1.}, backgroundScale{}, columnWidth{}, gutter{};
		std::int16_t attachment{}, direction{}, spacingStyle{}, columnType{};
		std::uint32_t backgroundFlags{}, backgroundColor{}, backgroundTransparency{}, extraHandles{};
		bool autoHeight{}, reversed{};
		std::vector<double> columnHeights;
		handle_t style{};
	};
	struct sText {
		point_t insertion, alignment;
		string_t text, tag, prompt;
		double height{}, widthFactor{1.}, rotation{}, oblique{};
		std::int16_t generation{}, horizontal{}, vertical{}, fieldLength{};
		std::uint8_t flags{}, version{}, attributeType{1};
		bool lockPosition{};
		handle_t style{};
		std::optional<sMText> multiline;
		std::uint32_t embeddedHandles{};
	};
	struct sPolyline {
		std::vector<point_t> points;
		std::vector<double> bulges;
		bool closed{};
		double constantWidth{};
		std::vector<std::pair<double, double>> widths;
		bool is3d{};
		enum class eKind { polyline, polyface, mesh };
		eKind kind{eKind::polyline};
		std::uint16_t countM{}, countN{}, densityM{}, densityN{};
		std::vector<sFaceRecord> faces;
		std::uint16_t flags{}, curveType{};
		double elevation{}, defaultStartWidth{}, defaultEndWidth{};
		handle_t firstVertex{}, lastVertex{}, sequenceEnd{};
		std::uint32_t ownedCount{};
		std::vector<handle_t> vertices; // R2004+ explicit ownership order.
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
		std::uint32_t ownedCount{};
		std::vector<handle_t> attributes;
		std::uint16_t columns{1}, rows{1};
		double columnSpacing{}, rowSpacing{};
	};
	struct sDimension {
		point_t textMidpoint, insertion, scale{1,1,1};
		std::vector<point_t> definitionPoints;
		string_t text;
		double textRotation{}, horizontalDirection{}, rotation{}, lineSpacing{}, measurement{}, extensionRotation{}, dimensionRotation{}, leaderLength{};
		std::uint8_t version{}, flags{}, ordinateFlags{};
		std::int16_t attachment{}, spacingStyle{};
		bool userTextPosition{}, flipFirst{}, flipSecond{};
		handle_t style{}, block{};
	};
	struct sHatch {
		struct sEdge { std::variant<sLine,sArc,sEllipse,sSpline> geometry; bool ccw{true}; };
		struct sPath { std::uint32_t flags{}, boundaryCount{}; sPolyline polyline; std::vector<sEdge> edges; std::vector<handle_t> boundaries; };
		struct sPatternLine { double angle{}; point_t base, offset; std::vector<double> dashes; };
		string_t name, gradientName;
		double elevation{}, angle{}, scale{}, pixelSize{}, gradientAngle{}, gradientShift{}, gradientTint{};
		bool solid{}, associative{}, doublePattern{};
		std::uint32_t gradient{}, reserved{}, singleColor{};
		std::int16_t style{}, patternType{};
		std::vector<std::pair<double,std::uint32_t>> gradientColors;
		std::vector<sPath> paths;
		std::vector<sPatternLine> patternLines;
		std::vector<point_t> seeds;
	};
	// monostate retains common metadata for structural and unsupported entities.
	using geometry_t = std::variant<std::monostate, sLine, sCircle, sArc, sPoint, sPolyline, sVertex, sInsert, sEllipse, sSpline, sText, sMText, sDimension, sHatch, sSolid, sFace3D, sRay, sShape, sFaceRecord>;
	struct sEntity {
		handle_t handle{}, owner{}, layer{}, previous{}, next{};
		std::uint16_t type{};
		std::uint8_t mode{}; // 0: owner reference, 1: paper space, 2: model space
		std::int16_t color{256};
		std::optional<std::uint32_t> rgb, transparency;
		handle_t colorBook{};
		std::uint8_t lineWeight{29}; // DWG enumeration, 29 = ByLayer
		bool invisible{};
		double lineTypeScale{1.}, thickness{};
		handle_t lineType{};
		std::uint8_t lineTypeMode{}; // 0 ByLayer, 1 ByBlock, 2 Continuous, 3 explicit.
		point_t extrusion{0., 0., 1.};
		geometry_t geometry;
	};

}

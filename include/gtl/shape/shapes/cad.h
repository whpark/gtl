#pragma once
// CAD types ported from biscuit.shape; GTL interfaces and Boost archives retained.
#include "../canvas.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace gtl::shape {
#pragma pack(push, 8)
struct sCadGroup {
	int16_t code{};
	boost::variant<bool, int16_t, int32_t, int64_t, double, string_t, std::vector<uint8_t>> value;
	bool operator==(sCadGroup const &) const = default;
	template <class Archive> void serialize(Archive &ar, unsigned) { ar & code & value; }
};
struct sCadBinary {
	string_t format;
	uint32_t version{}, codepage{}, objectType{};
	uint64_t handle{}, dataBitOffset{}, handleBitOffset{}, bitLength{};
	std::vector<uint8_t> bytes;
	bool operator==(sCadBinary const &) const = default;
	template <class Archive> void serialize(Archive &ar, unsigned) {
		ar & format & version & codepage & objectType & handle & dataBitOffset & handleBitOffset & bitLength & bytes;
	}
};
struct sCadText {
	point_t position{}, xAxis{1, 0, 0}, yAxis{0, 1, 0};
	string_t text;
	double height{2.5};
};
struct sCadGeometry {
	struct sLine {
		point_t pt0, pt1;
	};
	std::vector<sLine> lines;
	std::vector<sCadText> texts;
	bool Empty() const { return lines.empty() && texts.empty(); }
};
class GTL__SHAPE_CLASS xCadEntity : public xShape {
  public:
	template <class Archive> friend void serialize(Archive &ar, xCadEntity &value, unsigned version) {
		value.serialize(ar, version);
	}
	template <class Archive> friend Archive &operator&(Archive &ar, xCadEntity &value) {
		value.serialize(ar, 1);
		return ar;
	}
	string_t m_entityName;
	std::vector<sCadGroup> m_groups;
	std::optional<sCadBinary> m_binary;
	bool m_bExternalGraphics{}; // Importer emitted the stored display block.
	point_t m_origin{};
	std::array<point_t, 3> m_axes{point_t{1, 0, 0}, point_t{0, 1, 0}, point_t{0, 0, 1}};
	bool Compare(xShape const &other) const override {
		auto p = dynamic_cast<xCadEntity const *>(&other);
		return p && xShape::Compare(other) && m_entityName == p->m_entityName && m_groups == p->m_groups &&
			   m_binary == p->m_binary && m_bExternalGraphics == p->m_bExternalGraphics && m_origin == p->m_origin &&
			   m_axes == p->m_axes;
	}
	template <class Archive> void serialize(Archive &ar, unsigned version) {
		ar.operator&(boost::serialization::base_object<xShape>(*this));
		ar & m_entityName & m_groups & m_origin;
		for (auto &axis : m_axes)
			ar & axis;
		bool present = m_binary.has_value();
		ar & present;
		if constexpr (Archive::is_loading::value) {
			if (present)
				m_binary.emplace();
			else
				m_binary.reset();
		}
		if (present)
			ar &*m_binary;
		if (version >= 1)
			ar & m_bExternalGraphics;
		else if constexpr (Archive::is_loading::value)
			m_bExternalGraphics = false;
	}
	point_t ToWorld(point_t const &p) const { return m_origin + m_axes[0] * p.x + m_axes[1] * p.y + m_axes[2] * p.z; }
	std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override { return {}; }
	void Transform(xCoordTrans3d const &ct, bool) override {
		auto zero = ct(point_t{});
		for (auto &axis : m_axes)
			axis = ct(axis) - zero;
		m_origin = ct(m_origin);
	}
	void FlipX() override {
		m_origin.x = -m_origin.x;
		for (auto &axis : m_axes)
			axis.x = -axis.x;
	}
	void FlipY() override {
		m_origin.y = -m_origin.y;
		for (auto &axis : m_axes)
			axis.y = -axis.y;
	}
	void FlipZ() override {
		m_origin.z = -m_origin.z;
		for (auto &axis : m_axes)
			axis.z = -axis.z;
	}
	void Reverse() override {}
	sCadGeometry GetRenderGeometry() const;
	bool UpdateBoundary(rect_t &) const override;
	void Draw(ICanvas &) const override;
	bool DrawROI(ICanvas &, rect_t const &) const override;
	void PrintOut(std::wostream &os) const override {
		xShape::PrintOut(os);
		fmt::print(os, L"CAD: {}, {} groups, {} bytes\n", m_entityName, m_groups.size(),
				   m_binary ? m_binary->bytes.size() : 0);
	}
};
#define GTL_CAD_TYPE(CLASS, BASE, TYPE)                                                                                \
	using base_t = BASE;                                                                                               \
	GTL__DYNAMIC_VIRTUAL_DERIVED(CLASS);                                                                               \
	template <class Archive> friend void serialize(Archive &ar, CLASS &value, unsigned version) {                      \
		value.serialize(ar, version);                                                                                  \
	}                                                                                                                  \
	template <class Archive> friend Archive &operator&(Archive &ar, CLASS &value) {                                    \
		value.serialize(ar, 0);                                                                                        \
		return ar;                                                                                                     \
	}                                                                                                                  \
	eSHAPE GetShapeType() const override { return eSHAPE::TYPE; }
#define GTL_CAD_ENTITY(CLASS, TYPE)                                                                                    \
	class GTL__SHAPE_CLASS CLASS : public xCadEntity {                                                                 \
	  public:                                                                                                          \
		GTL_CAD_TYPE(CLASS, xCadEntity, TYPE);                                                                         \
		template <class Archive> void serialize(Archive &ar, unsigned) {                                               \
			ar.operator&(boost::serialization::base_object<xCadEntity>(*this));                                        \
		}                                                                                                              \
	};
GTL_CAD_ENTITY(xDimension, dimension)
GTL_CAD_ENTITY(xDimAligned, dimaligned)
GTL_CAD_ENTITY(xDimLinear, dimlinear)
GTL_CAD_ENTITY(xDimRadial, dimradial)
GTL_CAD_ENTITY(xDimDiametric, dimdiametric)
GTL_CAD_ENTITY(xDimAngular, dimangular)
GTL_CAD_ENTITY(xDimAngular3P, dimangular3p)
GTL_CAD_ENTITY(xDimOrdinate, dimordinate)
GTL_CAD_ENTITY(xAttDef, attdef)
GTL_CAD_ENTITY(xAttrib, attrib)
GTL_CAD_ENTITY(xLeader, leader)
GTL_CAD_ENTITY(xMLeader, mleader)
GTL_CAD_ENTITY(xTolerance, tolerance)
GTL_CAD_ENTITY(xImage, image)
GTL_CAD_ENTITY(xUnderlay, underlay)
GTL_CAD_ENTITY(xWipeout, wipeout)
GTL_CAD_ENTITY(xOleFrame, oleframe)
GTL_CAD_ENTITY(xOle2Frame, ole2frame)
GTL_CAD_ENTITY(xMLine, mline)
GTL_CAD_ENTITY(xHelix, helix)
GTL_CAD_ENTITY(x3DSolid, solid3d)
GTL_CAD_ENTITY(xBody, body)
GTL_CAD_ENTITY(xRegion, region)
GTL_CAD_ENTITY(xSurface, surface)
GTL_CAD_ENTITY(xMesh, mesh)
GTL_CAD_ENTITY(xTable, table)
GTL_CAD_ENTITY(xShapeEntity, shape)
GTL_CAD_ENTITY(xSection, section)
GTL_CAD_ENTITY(xViewport, viewport)
GTL_CAD_ENTITY(xLight, light)
GTL_CAD_ENTITY(xSun, sun)
GTL_CAD_ENTITY(xProxyEntity, proxy_entity)
#undef GTL_CAD_ENTITY
class GTL__SHAPE_CLASS x3DFace : public xShape {
  public:
	std::array<point_t, 4> m_pts{};
	uint16_t m_invisibleEdges{}; // Bits 0..3: corresponding edges are hidden.
	GTL_CAD_TYPE(x3DFace, xShape, e3dface);
	template <class Archive> void serialize(Archive &ar, unsigned) {
		ar.operator&(boost::serialization::base_object<xShape>(*this));
		for (auto &p : m_pts)
			ar & p;
		ar & m_invisibleEdges;
	}
	bool Compare(xShape const &other) const override {
		auto p = dynamic_cast<x3DFace const *>(&other);
		return p && xShape::Compare(other) && m_pts == p->m_pts && m_invisibleEdges == p->m_invisibleEdges;
	}
	std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override { return {}; }
	void FlipX() override {
		for (auto &p : m_pts)
			p.x = -p.x;
	}
	void FlipY() override {
		for (auto &p : m_pts)
			p.y = -p.y;
	}
	void FlipZ() override {
		for (auto &p : m_pts)
			p.z = -p.z;
	}
	void Reverse() override {
		std::swap(m_pts[1], m_pts[3]);
		auto f = m_invisibleEdges;
		m_invisibleEdges = (f & ~15u) | ((f & 1) << 3) | ((f & 2) << 1) | ((f & 4) >> 1) | ((f & 8) >> 3);
	}
	void Transform(xCoordTrans3d const &ct, bool) override {
		for (auto &p : m_pts)
			p = ct(p);
	}
	bool UpdateBoundary(rect_t &bounds) const override {
		bool changed{};
		for (auto const &p : m_pts)
			changed |= bounds.UpdateBoundary(p);
		return changed;
	}
	void Draw(ICanvas &canvas) const override {
		xShape::Draw(canvas);
		for (size_t i{}; i < 4; ++i)
			if (!(m_invisibleEdges & (1u << i)) && m_pts[i] != m_pts[(i + 1) % 4])
				canvas.Line(m_pts[i], m_pts[(i + 1) % 4]);
	}
};

// Corners are stored in perimeter order (DXF order is 0,1,3,2).
// Thickness is a vector so non-uniform transforms remain representable.
class GTL__SHAPE_CLASS xSolid : public x3DFace {
  public:
	point_t m_thickness{};
	GTL_CAD_TYPE(xSolid, x3DFace, solid);
	template <class Archive> void serialize(Archive &ar, unsigned) {
		ar.operator&(boost::serialization::base_object<x3DFace>(*this));
		ar & m_thickness;
	}
	bool Compare(xShape const &other) const override {
		auto p = dynamic_cast<xSolid const *>(&other);
		return p && x3DFace::Compare(other) && m_thickness == p->m_thickness;
	}
	void Transform(xCoordTrans3d const &ct, bool right) override {
		x3DFace::Transform(ct, right);
		m_thickness = ct(m_thickness) - ct(point_t{});
	}
	void FlipX() override {
		x3DFace::FlipX();
		m_thickness.x = -m_thickness.x;
	}
	void FlipY() override {
		x3DFace::FlipY();
		m_thickness.y = -m_thickness.y;
	}
	void FlipZ() override {
		x3DFace::FlipZ();
		m_thickness.z = -m_thickness.z;
	}
	bool UpdateBoundary(rect_t &bounds) const override {
		auto changed = x3DFace::UpdateBoundary(bounds);
		for (auto const &p : m_pts)
			changed |= bounds.UpdateBoundary(p + m_thickness);
		return changed;
	}
	void Draw(ICanvas &canvas) const override {
		x3DFace::Draw(canvas); // Wireframe; canvas has no polygon-fill primitive.
		if (m_thickness == point_t{})
			return;
		for (size_t i{}; i < 4; ++i) {
			canvas.Line(m_pts[i], m_pts[i] + m_thickness);
			canvas.Line(m_pts[i] + m_thickness, m_pts[(i + 1) % 4] + m_thickness);
		}
	}
};
class GTL__SHAPE_CLASS xTrace : public xSolid {
  public:
	GTL_CAD_TYPE(xTrace, xSolid, trace);
	template <class Archive> void serialize(Archive &ar, unsigned) {
		ar.operator&(boost::serialization::base_object<xSolid>(*this));
	}
	bool Compare(xShape const &other) const override {
		auto p = dynamic_cast<xTrace const *>(&other);
		return p && xSolid::Compare(other);
	}
};

class GTL__SHAPE_CLASS xRay : public xShape {
  public:
	point_t m_origin{}, m_direction{1., 0., 0.};
	GTL_CAD_TYPE(xRay, xShape, ray);
	template <class Archive> void serialize(Archive &ar, unsigned) {
		ar.operator&(boost::serialization::base_object<xShape>(*this));
		ar & m_origin;
		ar & m_direction;
	}
	bool Compare(xShape const &other) const override {
		auto p = dynamic_cast<xRay const *>(&other);
		return p && xShape::Compare(other) && m_origin == p->m_origin && m_direction == p->m_direction;
	}
	std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override { return {}; }
	void FlipX() override {
		m_origin.x = -m_origin.x;
		m_direction.x = -m_direction.x;
	}
	void FlipY() override {
		m_origin.y = -m_origin.y;
		m_direction.y = -m_direction.y;
	}
	void FlipZ() override {
		m_origin.z = -m_origin.z;
		m_direction.z = -m_direction.z;
	}
	void Reverse() override { m_direction *= -1.; }
	void Transform(xCoordTrans3d const &ct, bool) override {
		m_origin = ct(m_origin);
		m_direction = ct(m_direction) - ct(point_t{});
	}
	bool UpdateBoundary(rect_t &) const override { return false; } // Unbounded, excluded from auto-fit.
	std::optional<std::pair<point_t, point_t>> Clip(gtl::xRect2d const &bounds) const {
		auto finite = [](point_t const &p) { return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z); };
		if (!bounds.IsNormalized() || !finite(m_origin) || !finite(m_direction))
			return {};
		double lo = GetShapeType() == eSHAPE::xline ? -std::numeric_limits<double>::infinity() : 0.;
		double hi = std::numeric_limits<double>::infinity();
		auto slab = [&](double p, double d, double a, double b) {
			if (d == 0.)
				return p >= a && p <= b;
			auto t0 = (a - p) / d, t1 = (b - p) / d;
			if (t0 > t1)
				std::swap(t0, t1);
			lo = std::max(lo, t0);
			hi = std::min(hi, t1);
			return lo <= hi;
		};
		if (!slab(m_origin.x, m_direction.x, bounds.pt0().x, bounds.pt1().x) ||
			!slab(m_origin.y, m_direction.y, bounds.pt0().y, bounds.pt1().y) || !std::isfinite(lo) ||
			!std::isfinite(hi))
			return {};
		return std::pair{m_origin + m_direction * lo, m_origin + m_direction * hi};
	}
	void Draw(ICanvas &canvas) const override {
		if (!m_bVisible)
			return;
		if (auto roi = canvas.GetClippingRect()) {
			if (auto line = Clip(*roi)) {
				xShape::Draw(canvas);
				canvas.Line(line->first, line->second);
			}
		}
	}
	bool DrawROI(ICanvas &canvas, rect_t const &roi) const override {
		if (!m_bVisible)
			return false;
		gtl::xRect2d clip;
		clip.pt0() = xPoint2d{roi.pt0().x, roi.pt0().y};
		clip.pt1() = xPoint2d{roi.pt1().x, roi.pt1().y};
		if (auto canvasROI = canvas.GetClippingRect())
			clip = clip.IntersectRect(*canvasROI);
		if (auto line = Clip(clip)) {
			xShape::Draw(canvas);
			canvas.Line(line->first, line->second);
			return true;
		}
		return false;
	}
};
class GTL__SHAPE_CLASS xXLine : public xRay {
  public:
	GTL_CAD_TYPE(xXLine, xRay, xline);
	template <class Archive> void serialize(Archive &ar, unsigned) {
		ar.operator&(boost::serialization::base_object<xRay>(*this));
	}
	bool Compare(xShape const &other) const override {
		auto p = dynamic_cast<xXLine const *>(&other);
		return p && xRay::Compare(other);
	}
};
#undef GTL_CAD_TYPE
#pragma pack(pop)
} // namespace gtl::shape
BOOST_CLASS_VERSION(gtl::shape::xCadEntity, 1)
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDimension, "cad.dimension")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDimAligned, "cad.dimaligned")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDimLinear, "cad.dimlinear")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDimRadial, "cad.dimradial")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDimDiametric, "cad.dimdiametric")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDimAngular, "cad.dimangular")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDimAngular3P, "cad.dimangular3p")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDimOrdinate, "cad.dimordinate")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xAttDef, "cad.attdef")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xAttrib, "cad.attrib")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xLeader, "cad.leader")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xMLeader, "cad.mleader")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xTolerance, "cad.tolerance")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xImage, "cad.image")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xUnderlay, "cad.underlay")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xWipeout, "cad.wipeout")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xOleFrame, "cad.oleframe")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xOle2Frame, "cad.ole2frame")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xMLine, "cad.mline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xHelix, "cad.helix")
BOOST_CLASS_EXPORT_GUID(gtl::shape::x3DSolid, "cad.solid3d")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xBody, "cad.body")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xRegion, "cad.region")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xSurface, "cad.surface")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xMesh, "cad.mesh")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xTable, "cad.table")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xShapeEntity, "cad.shape")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xSection, "cad.section")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xViewport, "cad.viewport")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xLight, "cad.light")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xSun, "cad.sun")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xProxyEntity, "cad.proxy_entity")
BOOST_CLASS_EXPORT_GUID(gtl::shape::x3DFace, "cad.e3dface")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xSolid, "cad.solid")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xTrace, "cad.trace")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xRay, "cad.ray")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xXLine, "cad.xline")

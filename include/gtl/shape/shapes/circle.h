//////////////////////////////////////////////////////////////////////
//
// shape_others.h:
//
// PWH
// 2017.07.20
// 2021.05.27
// 2021.08.11 Compare
// 2023-06-09 분리
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../shape_primitives.h"
#include "../canvas.h"

//export module shape;

namespace gtl::shape {
#pragma pack(push, 8)

	class GTL__SHAPE_CLASS xCircle : public xShape {
	public:
		using base_t = xShape;
		using this_t = xCircle;

	public:
		point_t m_ptCenter;
		double m_radius{};
		deg_t m_angle_length{360_deg};	// 회전 방향.

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return (m_ptCenter == B.m_ptCenter) and (m_radius == B.m_radius) and (m_angle_length == B.m_angle_length);
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::circle_xy; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			auto pt = m_ptCenter+point_t{m_radius,0,0};
			return std::pair{ pt, pt };
		}
		virtual void FlipX() override { m_ptCenter.x = -m_ptCenter.x; m_angle_length = -m_angle_length; }
		virtual void FlipY() override { m_ptCenter.y = -m_ptCenter.y; m_angle_length = -m_angle_length; }
		virtual void FlipZ() override { m_ptCenter.z = -m_ptCenter.z; m_angle_length = -m_angle_length; }
		virtual void Reverse() override {
			m_angle_length = -m_angle_length;
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			m_ptCenter = ct(m_ptCenter); m_radius = ct.Trans(m_radius);
			if (!bRightHanded)
				m_angle_length = -m_angle_length;
		}
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool bResult{};
			bResult |= rectBoundary.UpdateBoundary(point_t(m_ptCenter.x-m_radius, m_ptCenter.y-m_radius, m_ptCenter.z));
			bResult |= rectBoundary.UpdateBoundary(point_t(m_ptCenter.x+m_radius, m_ptCenter.y+m_radius, m_ptCenter.z));
			return bResult;
		};
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Arc(m_ptCenter, m_radius, 0._deg, m_angle_length);
		}
		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			fmt::print(os, L"\tcenter({},{},{}), r {}\n", m_ptCenter.x, m_ptCenter.y, m_ptCenter.z, m_radius);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xCircle);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xCircle, xShape);
		//GTL__REFLECTION_MEMBERS(m_ptCenter, m_radius, m_angle_length);
		auto operator <=> (xCircle const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xCircle& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xCircle& var) {
			ar & boost::serialization::base_object<xShape>(var);
			return ar & var.m_ptCenter & var.m_radius & (double&)var.m_angle_length;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_ptCenter = PointFrom(j["basePoint"sv]);
			m_radius = j["radious"sv];
			return true;
		}
	};



#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xCircle, "circleXY")

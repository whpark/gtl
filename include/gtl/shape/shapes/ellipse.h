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
#include <numbers>

//export module shape;

namespace gtl::shape {
#pragma pack(push, 8)

	class GTL__SHAPE_CLASS xEllipse : public xArc {
	public:
		using base_t = xArc;
		using this_t = xEllipse;

	public:
		double m_radiusH{};
		deg_t m_angle_first_axis{};

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return (m_radiusH == B.m_radiusH) and (m_angle_first_axis == B.m_angle_first_axis);
		}
		virtual eSHAPE GetShapeType() const override { return eSHAPE::ellipse_xy; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			xCoordTrans2d ct;
			ct.Init(1.0, (rad_t)m_angle_first_axis, point_t{}, m_ptCenter);

			deg_t t0 = m_angle_start;
			deg_t t1 = m_angle_start + m_angle_length;
			auto radius1 = m_radius;
			auto radius2 = m_radiusH;
			point_t pt0 = ct(point_t{radius1*gtl::cos(t0), radius2*gtl::sin(t0)});
			point_t pt1 = ct(point_t{radius1*gtl::cos(t1), radius2*gtl::sin(t1)});

			return std::pair{ pt0, pt1 };
		}
		virtual void FlipX() override { xArc::FlipX(); m_angle_first_axis = 180._deg - m_angle_first_axis; }
		virtual void FlipY() override { xArc::FlipY(); m_angle_first_axis = - m_angle_first_axis; }
		virtual void FlipZ() override { xArc::FlipZ(); m_angle_first_axis = 180._deg - m_angle_first_axis; }
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			xArc::Transform(ct, bRightHanded);
			m_radiusH = ct.Trans(m_radiusH);
			if (!bRightHanded)
				m_angle_first_axis = -m_angle_first_axis;
		}
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool bResult{};
			auto c = gtl::cos(m_angle_first_axis), s = gtl::sin(m_angle_first_axis);
			double start = (double)(rad_t)m_angle_start, sweep = (double)(rad_t)m_angle_length;
			double const turn = 2.*std::numbers::pi;
			auto append = [&](double t) {
				bResult |= rectBoundary.UpdateBoundary(point_t{
					m_ptCenter.x + m_radius*c*std::cos(t) - m_radiusH*s*std::sin(t),
					m_ptCenter.y + m_radius*s*std::cos(t) + m_radiusH*c*std::sin(t), m_ptCenter.z});
			};
			append(start); append(start+sweep);
			// Coordinate extrema of the rotated parametric ellipse, restricted to the arc.
			for (double t : {std::atan2(-m_radiusH*s,m_radius*c), std::atan2(m_radiusH*c,m_radius*s)}) {
				for (int i=0; i<2; ++i, t+=std::numbers::pi) {
					double distance = std::fmod(sweep < 0. ? start-t : t-start, turn);
					if (distance < 0.) distance += turn;
					if (std::abs(sweep) >= turn || distance <= std::abs(sweep)) append(t);
				}
			}
			return bResult;
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Ellipse(m_ptCenter, m_radius, m_radiusH, m_angle_first_axis, m_angle_start, m_angle_length);
		}
		virtual void PrintOut(std::wostream& os) const override {
			xArc::PrintOut(os);
			fmt::print(os, L"\tradiusH:{}, angle_first_axis:{} deg\n", m_radiusH, (double)(deg_t)m_angle_first_axis);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xEllipse);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xEllipse, xArc);
		//GTL__REFLECTION_MEMBERS(m_radiusH, m_angle_first_axis);
		auto operator <=> (xEllipse const&) const = delete;
		bool operator == (xEllipse const& B) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xEllipse& var, unsigned int const file_version) {
			boost::serialization::base_object<xArc>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xEllipse& var) {
			ar & boost::serialization::base_object<xArc>(var);
			return ar & var.m_radiusH & (double&)var.m_angle_first_axis;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_ptCenter = PointFrom(j["basePoint"sv]);
			point_t firstAxis = PointFrom(j["secPoint"sv]);
			m_angle_first_axis = deg_t::atan2(firstAxis.y, firstAxis.x);
			double ratio = j["ratio"sv];
			m_radius = firstAxis.GetLength();
			m_radiusH = ratio * m_radius;

			rad_t t0 {(double)j["staparam"sv]};
			rad_t t1 {(double)j["endparam"sv]};
			bool bCCW = j["isccw"sv];

			m_angle_start = t0;
			m_angle_length = t1-t0;

			if (bCCW) {
				if (m_angle_length < 0_deg)
					m_angle_length += 360_deg;
			} else {
				if (m_angle_length > 0_deg)
					m_angle_length -= 360_deg;
			}

			return true;
		}

	};



#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xEllipse, "ellipseXY")

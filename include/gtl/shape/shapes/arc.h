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

	class GTL__SHAPE_CLASS xArc : public xCircle {
	public:
		using base_t = xCircle;
		using this_t = xArc;

	public:
		deg_t m_angle_start{};

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return m_angle_start == B.m_angle_start;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::arc_xy; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			auto c = gtl::cos(m_angle_start);
			auto s = gtl::sin(m_angle_start);
			auto pt0 = m_ptCenter + m_radius*point_t{c, s};
			c = gtl::cos(m_angle_start + m_angle_length);
			s = gtl::sin(m_angle_start + m_angle_length);
			auto pt1 = m_ptCenter + m_radius*point_t{c, s};
			return std::pair{ pt0, pt1 };
		}
		virtual void FlipX() override { xCircle::FlipX(); m_angle_start = AdjustAngle(180._deg - m_angle_start); }
		virtual void FlipY() override { xCircle::FlipY(); m_angle_start = AdjustAngle(-m_angle_start); }
		virtual void FlipZ() override { xCircle::FlipZ(); m_angle_start = AdjustAngle(180._deg - m_angle_start); }	// ????.....  성립 안되지만,
		virtual void Reverse() override {
			m_angle_start = m_angle_start+m_angle_length;
			m_angle_length = -m_angle_length;
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			// todo : ... upgrade?
			xCircle::Transform(ct, bRightHanded);
			if (!bRightHanded)
				m_angle_start = -m_angle_start;
		}
		auto At(auto t) const { return m_ptCenter + m_radius * point_t{gtl::cos(t), gtl::sin(t)}; };
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool bResult{};
			// todo : ... upgrade?
			rect_t rectMax(m_ptCenter, m_ptCenter);
			rectMax.pt0() -= point_t{m_radius, m_radius};
			rectMax.pt1() += point_t{m_radius, m_radius};
			if (rectBoundary.RectInRect(rectMax))
				return bResult;
			auto start = m_angle_start;
			if (start < 0_deg)
				start += 360_deg;
			auto end = start + m_angle_length;
			bResult |= rectBoundary.UpdateBoundary(At(start));
			bResult |= rectBoundary.UpdateBoundary(At(end));
			if (start > end)
				std::swap(start, end);
			int count{};
			int iend = end;
			for (int t = (int)std::floor(deg_t(start.dValue/90_deg))*90+90; t <= iend; t += 90) {
				switch (t%360) {
				case 0 :		bResult |= rectBoundary.UpdateBoundary(m_ptCenter + point_t{m_radius, 0.}); break;
				case 90 :		bResult |= rectBoundary.UpdateBoundary(m_ptCenter + point_t{0., m_radius}); break;
				case 180 :		bResult |= rectBoundary.UpdateBoundary(m_ptCenter + point_t{-m_radius, 0.}); break;
				case 270 :		bResult |= rectBoundary.UpdateBoundary(m_ptCenter + point_t{0., -m_radius}); break;
				}
				if (count++ >=4)
					break;
			}
			return bResult;
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Arc(m_ptCenter, m_radius, m_angle_start, m_angle_length);
		}
		virtual void PrintOut(std::wostream& os) const override {
			xCircle::PrintOut(os);
			fmt::print(os, L"\tangle_start:{} deg, length:{} deg\n", (double)(deg_t)m_angle_start, (double)(deg_t)m_angle_length);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xArc);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xArc, xCircle);
		//GTL__REFLECTION_MEMBERS(m_angle_start);
		auto operator <=> (xArc const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xArc& var, unsigned int const file_version) {
			boost::serialization::base_object<xCircle>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xArc& var) {
			ar & boost::serialization::base_object<xCircle>(var);
			return ar & var.m_angle_start;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xCircle::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_angle_start = deg_t{(double)j["staangle"sv]};
			bool bCCW = j["isccw"sv].value_or(0) != 0;
			deg_t angle_end { (double)j["endangle"sv] };
			m_angle_length = angle_end - m_angle_start;
			if (bCCW) {
				if (m_angle_length < 0_deg)
					m_angle_length += 360_deg;
			} else {
				if (m_angle_length > 0_deg)
					m_angle_length -= 360_deg;
			}
			return true;
		}

		deg_t AdjustAngle(deg_t angle) const {
			//if ( (angle < 0) || (angle > 360.) ) {
			//	int r = angle / 360.;
			//	angle = angle - 360. * r;
			//}
			//while (angle < 0)
			//	angle += 360.;
			angle = std::fmod(angle, 360.);
			if (angle < 0.)
				angle += 360.;
			return angle;
		}

		static xArc GetFromBulge(double bulge, point_t const& pt0, point_t const& pt1) {
			xArc arc;
			xPoint2d vecPerpendicular(-(pt0.y-pt1.y), (pt0.x-pt1.x));
			// Normalize
			{
				double d = vecPerpendicular.Distance(xPoint2d(0.0, 0.0));
				vecPerpendicular.x /= d;
				vecPerpendicular.y /= d;
			}
			xPoint2d ptCenterOfLine((pt0.x+pt1.x)/2., (pt0.y+pt1.y)/2.);
			double l = pt1.Distance(pt0)/2.;
			xPoint2d ptBulge;
			ptBulge.x = ptCenterOfLine.x + vecPerpendicular.x * (bulge * l);
			ptBulge.y = ptCenterOfLine.y + vecPerpendicular.y * (bulge * l);
			double h = ptBulge.Distance(ptCenterOfLine);
			arc.m_radius = (Square(l) + Square(h)) / (2 * h);

			arc.m_ptCenter.x = ptBulge.x + (arc.m_radius / h) * (ptCenterOfLine.x - ptBulge.x);
			arc.m_ptCenter.y = ptBulge.y + (arc.m_radius / h) * (ptCenterOfLine.y - ptBulge.y);
			arc.m_angle_start = rad_t::atan2(pt0.y - arc.m_ptCenter.y, pt0.x - arc.m_ptCenter.x);
			rad_t dT1 = rad_t::atan2(pt1.y - arc.m_ptCenter.y, pt1.x - arc.m_ptCenter.x);
			//arc.m_eDirection = (dBulge > 0) ? 1 : -1;
			//arc.m_dTLength = (dBulge > 0) ? fabs(dT1-arc.m_dT0) : -fabs(dT1-arc.m_dT0);
			if (bulge > 0) {
				while (dT1 < arc.m_angle_start)
					dT1 += rad_t(std::numbers::pi*2);
				arc.m_angle_length = dT1 - arc.m_angle_start;
			} else {
				while (dT1 > arc.m_angle_start)
					dT1 -= rad_t(std::numbers::pi*2);
				arc.m_angle_length = dT1 - arc.m_angle_start;
			}

			return arc;
		}

	};



#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xArc, "arcXY")

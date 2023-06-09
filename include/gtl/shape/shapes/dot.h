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

	class GTL__SHAPE_CLASS xDot : public xShape {
	public:
		using base_t = xShape;
		using this_t = xDot;

	public:
		point_t m_pt;

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return (m_pt == B.m_pt);
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::dot; }

		//virtual point_t PointAt(double t) const override { return pt; };
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			return std::pair{ m_pt, m_pt };
		}
		virtual void FlipX() override { m_pt.x = -m_pt.x; }
		virtual void FlipY() override { m_pt.y = -m_pt.y; }
		virtual void FlipZ() override { m_pt.z = -m_pt.z; }
		virtual void Reverse() override {
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			m_pt = ct(m_pt);
		};
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			return rectBoundary.UpdateBoundary(m_pt);
		};
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.MoveTo(m_pt);
			canvas.LineTo(m_pt);
		}
		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			fmt::print(os, L"\tpt({},{},{})\n", m_pt.x, m_pt.y, m_pt.z);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xDot);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xDot, xShape);
		//GTL__REFLECTION_MEMBERS(pt);
		auto operator <=> (xDot const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xDot& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xDot& var) {
			ar & boost::serialization::base_object<xShape>(var);
			ar & var.m_pt;
			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);
			m_pt = PointFrom(j["basePoint"sv]);
			return true;
		}

	};


#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xDot, "dot")

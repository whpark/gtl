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

	class GTL__SHAPE_CLASS xLine : public xShape {
	public:
		using base_t = xShape;
		using this_t = xLine;

	public:
		point_t m_pt0, m_pt1;

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return (m_pt0 == B.m_pt0) and (m_pt1 == B.m_pt1);
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::line; }

		//virtual point_t PointAt(double t) const override { return lerp(m_pt0, m_pt1, t); }
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			return std::pair{ m_pt0, m_pt1 };
		}
		virtual void FlipX() override { m_pt0.x = -m_pt0.x; m_pt1.x = -m_pt1.x; }
		virtual void FlipY() override { m_pt0.y = -m_pt0.y; m_pt1.y = -m_pt1.y; }
		virtual void FlipZ() override { m_pt0.z = -m_pt0.z; m_pt1.z = -m_pt1.z; }
		virtual void Reverse() override {
			std::swap(m_pt0, m_pt1);
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			m_pt0 = ct(m_pt0); m_pt1 = ct(m_pt1);
		};
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool bModified{};
			bModified |= rectBoundary.UpdateBoundary(m_pt0);
			bModified |= rectBoundary.UpdateBoundary(m_pt1);
			return bModified;
		};
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Line(m_pt0, m_pt1);
		}
		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			fmt::print(os, L"\tpt0({},{},{}) - pt1({},{},{})\n", m_pt0.x, m_pt0.y, m_pt0.z, m_pt1.x, m_pt1.y, m_pt1.z);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xLine);
		auto operator <=> (xLine const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xLine& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xLine& var) {
			ar & boost::serialization::base_object<xShape>(var);
			return ar & var.m_pt0 & var.m_pt1;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);
			m_pt0 = PointFrom(j["basePoint"sv]);
			m_pt1 = PointFrom(j["secPoint"sv]);
			return true;
		}
	};


#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xLine, "line")

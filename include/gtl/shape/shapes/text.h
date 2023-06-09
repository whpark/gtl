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

	class GTL__SHAPE_CLASS xText : public xShape {
	public:
		using base_t = xShape;
		using this_t = xText;

	public:
		enum class eALIGN_VERT : int { base_line = 0, bottom, mid, top };
		enum class eALIGN_HORZ : int { left = 0, center, right, aligned, middle, fit };

		point_t m_pt0, m_pt1;
		string_t m_text;
		double m_height{};
		deg_t m_angle{};
		double m_widthScale{};
		deg_t m_oblique{};
		string_t m_textStyle;
		int m_textgen{};
		eALIGN_HORZ m_alignHorz{eALIGN_HORZ::left};
		eALIGN_VERT m_alignVert{eALIGN_VERT::base_line};

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and ( m_pt0			== B.m_pt0 )
				and ( m_pt1			== B.m_pt1 )
				and ( m_text		== B.m_text )
				and ( m_height		== B.m_height )
				and ( m_angle		== B.m_angle )
				and ( m_widthScale	== B.m_widthScale )
				and ( m_oblique		== B.m_oblique )
				and ( m_textStyle	== B.m_textStyle )
				and ( m_textgen		== B.m_textgen )
				and ( m_alignHorz	== B.m_alignHorz )
				and ( m_alignVert	== B.m_alignVert )
				;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::text; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			// todo : Start/End Point
			return std::pair{ m_pt0, m_pt1 };
		}
		virtual void FlipX() override { m_pt0.x = -m_pt0.x; m_pt1.x = -m_pt1.x; }
		virtual void FlipY() override { m_pt0.y = -m_pt0.y; m_pt1.y = -m_pt1.y; }
		virtual void FlipZ() override { m_pt0.z = -m_pt0.z; m_pt1.z = -m_pt1.z; }
		virtual void Reverse() override {
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			m_pt0 = ct(m_pt0);
			m_pt1 = ct(m_pt1);
		}
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			// todo : upgrade.
			bool b{};
			b |= rectBoundary.UpdateBoundary(m_pt0);
			//b |= rectBoundary.UpdateBoundary(pt1);
			return b;
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Text(*this);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xText);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xText, xShape);
		//GTL__REFLECTION_MEMBERS(pt0, pt1, text, height, angle, widthScale, oblique, textStyle, textgen, alignHorz, alignVert);
		auto operator <=> (xText const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xText& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xText& var) {
			ar & boost::serialization::base_object<xShape>(var);

			ar & var.m_pt0 & var.m_pt1;
			ar & var.m_text;
			ar & var.m_height;
			ar & var.m_angle;
			ar & var.m_widthScale;
			ar & var.m_oblique;
			ar & var.m_textStyle;
			ar & var.m_textgen;
			ar & (int&)var.m_alignHorz;
			ar & (int&)var.m_alignVert;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_pt0 = PointFrom(j["basePoint"sv]);
			m_pt1 = PointFrom(j["secPoint"sv]);
			m_text = j["text"sv];
			m_height = j["height"sv];
			m_angle = deg_t{(double)j["angle"sv]};
			m_widthScale = j["widthscale"sv];
			m_oblique = deg_t{(double)j["oblique"sv]};
			m_textStyle = j["style"sv];
			m_textgen = j["textgen"sv];
			m_alignHorz = (eALIGN_HORZ)(int)j["alignH"sv];
			m_alignVert = (eALIGN_VERT)(int)j["alignV"sv];

			return true;
		}
	};

	class GTL__SHAPE_CLASS xMText : public xText {
	public:
		using base_t = xText;
		using this_t = xMText;

	protected:
		using xText::m_alignVert;
	public:
		double m_interlin{};
		enum class eATTACH {
			topLeft = 1,
			topCenter,
			topRight,
			middleLeft,
			middleCenter,
			middleRight,
			bottomLeft,
			bottomCenter,
			bottomRight
		};

		//eATTACH& eAttch{(eATTACH&)alignVert};
		eATTACH GetAttachPoint() const { return (eATTACH)m_alignVert; }
		void SetAttachPoint(eATTACH eAttach) { m_alignVert = (eALIGN_VERT)eAttach; }

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return m_interlin == B.m_interlin;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::mtext; }

		//virtual point_t PointAt(double t) const override {};
		//virtual void FlipX() override {}
		//virtual void FlipY() override {}
		//virtual void FlipY() override {}
		//virtual void Transform(xCoordTrans3d const&, bool bRightHanded) override { return true; };
		//virtual bool UpdateBoundary(rect_t& rectBoundary) const override { return true; };
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Text(*this);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xMText);
		auto operator <=> (xMText const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xMText& var, unsigned int const file_version) {
			boost::serialization::base_object<xText>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xMText& var) {
			ar & boost::serialization::base_object<xText>(var);
			return ar & var.m_interlin;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xText::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_interlin = j["interlin"sv];

			return true;
		}
	};


#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xText, "text")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xMText, "mtext")

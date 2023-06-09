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

///////////////////////////////////////////////////////////////////////////////
/*

	1. CShapeObject 에서 변경 내용 :

		class name :

			CShapeObject	-> xShape
			CShapeLine		-> xLine
			CShapePolyLine	-> xPolyline
			...

		Container :

			TList -> boost::ptr_deque

		Point / Line : x, y, z 3차원으로 변경

			typedef xSize3d					size2d_t;
			typedef xPoint3d				point_t;
			typedef xRect3d					rect_t;
			typedef std::vector<xLine>		s_lines;

*/
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gtl/iconv_wrapper.h"

#include "shape_primitives.h"
#include "canvas.h"

#include "shapes/layer.h"
#include "shapes/dot.h"
#include "shapes/line.h"
#include "shapes/circle.h"
#include "shapes/arc.h"
#include "shapes/ellipse.h"
#include "shapes/polyline.h"
#include "shapes/spline.h"
#include "shapes/text.h"
#include "shapes/block.h"
#include "shapes/drawing.h"

//export module shape;

namespace gtl::shape {
#pragma pack(push, 8)

	class GTL__SHAPE_CLASS xHatch : public xShape {
	public:
		using base_t = xShape;
		using this_t = xHatch;

	public:
		string_t m_name;
		bool m_bSolid{};
		bool m_bAssociative{};
		int m_nLoops{};
		int m_hstyle{};
		int m_hpattern{};
		bool m_bDouble{};
		deg_t m_angle{};
		double m_scale{};
		int m_deflines{};

		std::vector<xPolyline> m_boundaries;

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and ( m_name			== B.m_name )
				and ( m_bSolid			== B.m_bSolid )
				and ( m_bAssociative	== B.m_bAssociative )
				and ( m_nLoops			== B.m_nLoops )
				and ( m_hstyle			== B.m_hstyle )
				and ( m_hpattern		== B.m_hpattern )
				and ( m_bDouble			== B.m_bDouble )
				and ( m_angle			== B.m_angle )
				and ( m_scale			== B.m_scale )
				and ( m_deflines		== B.m_deflines )
				;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::hatch; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			return {};
		}
		virtual void FlipX() override { for (auto& b : m_boundaries) b.FlipX(); }
		virtual void FlipY() override { for (auto& b : m_boundaries) b.FlipY(); }
		virtual void FlipZ() override { for (auto& b : m_boundaries) b.FlipZ(); }
		virtual void Reverse() override {
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
		};
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool b{};
			for (auto const& bound : m_boundaries) {
				b |= bound.UpdateBoundary(rectBoundary);
			}
			return b;
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			// todo : draw hatch
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xHatch);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xHatch, xShape);
		//GTL__REFLECTION_MEMBERS(name, bSolid, bAssociative, nLoops, hstyle, hpattern, bDouble, angle, scale, deflines);
		auto operator <=> (xHatch const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xHatch& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xHatch& var) {
			ar & boost::serialization::base_object<xShape>(var);

			ar & var.m_name;
			ar & var.m_bSolid;
			ar & var.m_bAssociative;
			ar & var.m_nLoops;
			ar & var.m_hstyle;
			ar & var.m_hpattern;
			ar & var.m_angle;
			ar & var.m_scale;
			ar & var.m_deflines;
			ar & var.m_boundaries;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_name			= j["name"sv];
			m_bSolid		= (bool)(int)j["solid"sv];
			m_bAssociative	= (bool)(int)j["associative"sv];
			m_hstyle		= j["hstyle"sv];
			m_hpattern		= j["hpattern"sv];
			m_bDouble		= (bool)(int)j["doubleflag"sv];
			m_nLoops		= j["loopsnum"sv];
			m_angle			= rad_t{(double)j["angle"sv]};
			m_scale			= j["scale"sv];
			m_deflines		= j["deflines"sv];

			return true;
		}

	};

	// temporary object
	class GTL__SHAPE_CLASS xInsert : public xShape {
	public:
		using base_t = xShape;
		using this_t = xInsert;

	public:
		string_t m_name;	// block name
		point_t m_pt;
		double m_xscale{1};
		double m_yscale{1};
		double m_zscale{1};
		rad_t m_angle{};
		int m_nCol{1};
		int m_nRow{1};
		double m_spacingCol{};
		double m_spacingRow{};

		GTL__DYNAMIC_VIRTUAL_DERIVED(xInsert);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xInsert, xShape);
		//GTL__REFLECTION_MEMBERS(name, xscale, yscale, zscale, angle, nCol, nRow, spacingCol, spacingRow);
		auto operator <=> (xInsert const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xInsert& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xInsert& var) {
			ar & var.m_name & var.m_xscale & var.m_yscale & var.m_zscale & (double&)m_angle;
			ar & var.m_nCol & var.m_nRow & var.m_spacingCol & var.m_spacingRow;
			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			using namespace std::literals;
			xShape::LoadFromCADJson(_j);
			gtl::bjson j(_j);

			m_pt = PointFrom(j["basePoint"]);
			m_name = j["name"];
			m_xscale = j["xscale"];
			m_yscale = j["yscale"];
			m_zscale = j["zscale"];
			m_angle = rad_t{(double)j["angle"]};
			m_nCol = j["colcount"];
			m_nRow = j["rowcount"];
			m_spacingCol = j["colspace"];
			m_spacingRow = j["rowspace"];

			return true;
		}

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and ( m_name		== B.m_name )
				and ( m_pt			== B.m_pt )
				and ( m_xscale		== B.m_xscale )
				and ( m_yscale		== B.m_yscale )
				and ( m_zscale		== B.m_zscale )
				and ( m_angle		== B.m_angle )
				and ( m_nCol		== B.m_nCol )
				and ( m_nRow		== B.m_nRow )
				and ( m_spacingCol	== B.m_spacingCol )
				and ( m_spacingRow	== B.m_spacingRow )
				;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::insert; }

		//virtual point_t PointAt(double t) const = 0;
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override { return {}; }
		virtual void FlipX() override {}
		virtual void FlipY() override {}
		virtual void FlipZ() override {}
		virtual void Reverse() override {}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded /*= ct.IsRightHanded()*/) override {};
		virtual bool UpdateBoundary(rect_t&) const override { return false; };
		virtual void Draw(ICanvas& canvas) const override {
			//xShape::Draw(canvas);
		};

		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			fmt::print(os, L"\tname:{}, pt:({},{},{}), scale:({},{},{}), angle:{} deg, nCol:{}, nRow:{}, SpaceCol:{}, SpaceRow:{}\n",
					   m_name, m_pt.x, m_pt.y, m_pt.z, m_xscale, m_yscale, m_zscale, (double)(deg_t)m_angle, m_nCol, m_nRow, m_spacingCol, m_spacingRow);
		}
	};


#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xHatch, "hatch")

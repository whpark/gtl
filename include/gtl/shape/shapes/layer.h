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

	class GTL__SHAPE_CLASS xLayer : public xShape {
	public:
		using base_t = xShape;
		using this_t = xLayer;

	public:
		string_t m_name;
		bool m_bUse{true};
		int m_flags{};
		boost::ptr_deque<xShape> m_shapes;

		xLayer() = default;
		xLayer(xLayer const&) = default;
		xLayer(xLayer&&) = default;
		xLayer(string_t const& name) : m_name(name) {}

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and (m_name == B.m_name)
				and (m_bUse == B.m_bUse)
				and (m_flags == B.m_flags)
				and (m_shapes == B.m_shapes);
		}

		virtual eSHAPE GetShapeType() const { return eSHAPE::layer; }
		//virtual point_t PointAt(double t) const override { throw std::exception{"not here."}; return point_t {}; }	// no PointAt();
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			if (m_shapes.empty())
				return{};
			auto r0 = m_shapes.front().GetStartEndPoint();
			if (!r0)
				return {};
			auto r1 = m_shapes.back().GetStartEndPoint();
			if (!r1)
				return {};
			return std::pair{r0->first, r1->second};
		}
		virtual void FlipX() override { for (auto& shape : m_shapes) shape.FlipX(); }
		virtual void FlipY() override { for (auto& shape : m_shapes) shape.FlipY(); }
		virtual void FlipZ() override { for (auto& shape : m_shapes) shape.FlipZ(); }
		virtual void Reverse() override {
			std::ranges::reverse(m_shapes.base());
			for (auto& shape : m_shapes) {
				shape.Reverse();
			}
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			for (auto& shape : m_shapes)
				shape.Transform(ct, bRightHanded);
		}
		virtual bool UpdateBoundary(rect_t& rect) const override {
			bool r{};
			for (auto& shape : m_shapes)
				r |= shape.UpdateBoundary(rect);
			return r;
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			for (auto& shape : m_shapes) {
				shape.Draw(canvas);
			}
		}
		virtual bool DrawROI(ICanvas& canvas, rect_t const& rectROI) const override {
			bool result{};
			for (auto& shape : m_shapes) {
				result |= shape.DrawROI(canvas, rectROI);
			}
			return result;
		}
		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			for (auto& shape : m_shapes) {
				shape.PrintOut(os);
			}
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xLayer);
		//GTL__REFLECTION_DERIVED(xLayer, xShape);
		//GTL__REFLECTION_MEMBERS(name, flags, m_strLineType, m_lineWeight, shapes);
		auto operator <=> (xLayer const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xLayer& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xLayer& var) {
			ar & boost::serialization::base_object<xShape>(var);

			ar & var.m_name & var.m_bUse &var.m_flags;
			ar & var.m_strLineType & var.m_lineWeight;	// duplicated.... hot to delete ?
			ar & var.m_shapes;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);
			m_name = j["name"];	// not a "layer"
			m_flags = j["flags"];
			m_bUse = j["plotF"];

			return true;
		}

		void clear() {
			m_shapes.clear();
			m_strLineType.clear();
			m_flags = {};
			m_bUse = true;
			m_name.clear();
		}

		virtual void Sort_Loop();
		bool IsLoop(double dMinGap = 1.e-3) const;

		inline static bool RemoveConnectedComponents(std::deque<xShape const*>& shapes, xShape const* pSeed, double const dMinDistance = 0.000'1) {
			if (!pSeed)
				return false;

			std::deque<xShape const*> lst_found;
			lst_found.push_back(pSeed);

			while (lst_found.size()) {

				auto* p0 = lst_found.front();
				lst_found.pop_front();

				auto r = p0->GetStartEndPoint();
				if (!r)
					continue;
				auto [pt0, pt1] = *r;

				for (size_t i{}; i < shapes.size(); i++) {
					auto const* pNext = shapes[i];
					auto pts = pNext->GetStartEndPoint();
					if (!pts)
						continue;
					if ((pt0.Distance(pts->first) <= dMinDistance) or (pt0.Distance(pts->second) <= dMinDistance) or
						(pt1.Distance(pts->first) <= dMinDistance) or (pt1.Distance(pts->second) <= dMinDistance)) {
						lst_found.push_back(pNext);
						shapes.erase(shapes.begin()+i--);
					}
				}

				if (shapes.empty())
					break;
			}

			return true;
		}

	protected:
		friend class xDrawing;
		line_type_t* pLineType{};

	};



#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xLayer, "layer")

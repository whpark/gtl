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

	class GTL__SHAPE_CLASS xDrawing : public xShape {
	public:
		using base_t = xShape;
		using this_t = xDrawing;

	public:
		std::map<std::string, variable_t> m_vars;
		boost::ptr_deque<line_type_t> m_line_types;
		//boost::ptr_deque<xBlock> blocks;
		rect_t m_rectBoundary;
		boost::ptr_deque<xLayer> m_layers;

		//xDrawing() = default;
		//xDrawing(xDrawing const&) = default;
		//xDrawing(xDrawing &&) = default;
		//xDrawing& operator = (xDrawing const&) = default;
		//xDrawing& operator = (xDrawing &&) = default;

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and ( m_vars			== B.m_vars )
				and ( m_line_types		== B.m_line_types )
				and ( m_rectBoundary	== B.m_rectBoundary )
				and ( m_layers			== B.m_layers )
				;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::drawing; }

		//virtual point_t PointAt(double t) const override { throw std::exception{GTL__FUNCSIG "not here."}; return point_t {}; }	// no PointAt();
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			if (m_layers.empty())
				return {};
			auto r0 = m_layers.front().GetStartEndPoint();
			if (!r0)
				return {};
			auto r1 = m_layers.back().GetStartEndPoint();
			if (!r1)
				return {};
			return std::pair{ r0->first, r1->second };
		}
		virtual void FlipX() override { for (auto& layer : m_layers) layer.FlipX(); }
		virtual void FlipY() override { for (auto& layer : m_layers) layer.FlipY(); }
		virtual void FlipZ() override { for (auto& layer : m_layers) layer.FlipZ(); }
		virtual void Reverse() override {
			std::ranges::reverse(m_layers.base());
			for (auto& layer : m_layers) {
				layer.Reverse();
			}
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			for (auto& layer : m_layers)
				layer.Transform(ct, bRightHanded);
		}
		virtual bool UpdateBoundary(rect_t& rect) const override {
			bool r{};
			for (auto& layer : m_layers)
				r |= layer.UpdateBoundary(rect);
			return r;
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			for (auto& layer : m_layers) {
				layer.Draw(canvas);
			}
		}
		virtual bool DrawROI(ICanvas& canvas, rect_t const& rectROI) const override {
			bool result{};
			for (auto& layer : m_layers) {
				result |= layer.DrawROI(canvas, rectROI);
			}
			return result;
		}
		virtual void PrintOut(std::wostream& os) const override {
			for (auto& layer : m_layers) {
				layer.PrintOut(os);
			}
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xDrawing);
		//GTL__REFLECTION_DERIVED(xDrawing, xShape);
		//GTL__REFLECTION_MEMBERS(vars, line_types, layers);
	#if 0
		bool operator == (xDrawing const& B) const {
			if ((base_t const&)*this != (base_t const&)B)
				return false;
			bool bEQ = (m_vars == B.m_vars);
			bEQ &= (m_line_types == B.m_line_types);
			bEQ &= (m_rectBoundary == B.m_rectBoundary);
			bEQ &= (m_layers == B.m_layers);
			if (m_layers.size() == B.m_layers.size()) {
				for (size_t i{}; i < m_layers.size(); i++) {
					auto const& layerA = m_layers[i];
					auto const& layerB = B.m_layers[i];
					if (layerA.m_shapes.size() != layerB.m_shapes.size())
						return false;
					for (size_t iShape{}; iShape < layerA.m_shapes.size(); iShape++) {
						auto const& shapeA = layerA.m_shapes[iShape];
						auto const& shapeB = layerB.m_shapes[iShape];
						if ((xShape const&)shapeA != (xShape const&)shapeB)
							return false;
						if (shapeA != shapeB)
							return false;
					}
					if (layerA != layerB)
						return false;
				}
			}
			if (!bEQ)
				return false;
			return true;
		}
	#endif
		auto operator <=> (xDrawing const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xDrawing& var, unsigned int const file_version) {
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xDrawing& var) {
			ar & boost::serialization::base_object<xShape>(var);

			ar & var.m_vars;
			ar & var.m_line_types;
			//ar & var.blocks;
			ar & var.m_rectBoundary;
			ar & var.m_layers;

			return ar;
		}

		xLayer& Layer(string_t const& name) {
			if (auto iter = std::find_if(m_layers.begin(), m_layers.end(), [&name](xLayer const& layer) -> bool { return name == layer.m_name; }); iter != m_layers.end())
				return *iter;
			return m_layers.front();
		}
		xLayer const& Layer(string_t const& name) const {
			if (auto iter = std::find_if(m_layers.begin(), m_layers.end(), [&name](xLayer const& layer) -> bool { return name == layer.m_name; }); iter != m_layers.end())
				return *iter;
			return m_layers.front();
		}

		bool AddEntity(std::unique_ptr<xShape> rShape, std::map<string_t, xLayer*> const& mapLayers, std::map<string_t, xBlock*> const& mapBlocks, rect_t& rectBoundary);

	public:
		virtual bool LoadFromCADJson(json_t& _j) override;

		void clear() {
			m_vars.clear();
			m_line_types.clear();
			m_layers.clear();
		}
	};


#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xDrawing, "drawing")

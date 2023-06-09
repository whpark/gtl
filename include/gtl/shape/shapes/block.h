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

	// internally
	class GTL__SHAPE_CLASS xBlock : public xLayer {
	public:
		using base_t = xLayer;
		using this_t = xBlock;

	public:
		string_t m_layer;
		point_t m_pt;

	public:
		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and (m_layer == B.m_layer)
				and (m_pt == B.m_pt)
				;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::block; }

		GTL__DYNAMIC_VIRTUAL_DERIVED(xBlock);
		auto operator <=> (xBlock const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xBlock& var, unsigned int const file_version) {
			boost::serialization::base_object<xLayer>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xBlock& var) {
			ar & boost::serialization::base_object<xLayer>(var);

			ar & var.m_layer & var.m_pt;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			this->m_layer = m_layer;
			using namespace std::literals;
			gtl::bjson j(_j);

			m_name = j["name"];
			m_flags = j["flags"];
			m_pt = PointFrom(j["basePoint"]);

			return true;
		}
	};

#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xBlock, "block")

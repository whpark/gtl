#pragma once

//////////////////////////////////////////////////////////////////////
//
// macro.h : gtl.dxf internal macros
//
// PWH
// 2026-09-02 biscuit.dxf/macro.h -> gtl.dxf
//
//////////////////////////////////////////////////////////////////////

#define GTL__DXF_DEFINE_SPACESHIP_OPERATOR(CLASS_NAME) \
	auto operator <=> (CLASS_NAME const&) const = default;\
	bool operator == (CLASS_NAME const&) const = default;

#define GTL__DXF_ENTITY_DERIVED(tEntity, eEntityType, NAME, tSubclasses)\
	template class TEntityDerived<eEntityType, NAME, tSubclasses>;\
	using tEntity = TEntityDerived<eEntityType, NAME, tSubclasses>;


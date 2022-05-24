#pragma once


//////////////////////////////////////////////////////////////////////
//
// reflection.h: reflection for streaming, dynamic creating (json/archive...)
//
// PWH.
// 2020.01.03.
// 2021.01.04. member_wise.h -> reflection.h
// 2021.01.17. json proxy
//
//////////////////////////////////////////////////////////////////////


#include "gtl/_default.h"
#include "gtl/misc.h"
#include "gtl/string.h"
#include "gtl/json_proxy.h"


namespace gtl {
#pragma pack(push, 8)

	// REFL : Reflection. (Member Wise..., variables only. not for function ptr.)


#define I_GTL__REFLECTION_MEMBER(var)	gtl::internal::pair{ std::string_view{#var}, &this_t::var }

#define GTL__REFLECTION_MEMBERS(...) \
	constexpr static inline const std::tuple s_member_tuple {\
		GTL__RECURSIVE_MACRO_COMMA(I_GTL__REFLECTION_MEMBER, __VA_ARGS__)\
	};


	//================================================================================================================================
	// reflection - MACRO version
	//


#define GTL__REFLECTION_BASE(TJSON)\
	using reflection_base_t = this_t;\
	using json_t = TJSON;\
	template < typename tjson >\
	friend void from_json(tjson const& j, this_t& var) {\
		std::apply([&j, &var](auto& ... args) { ((var.*(args.second) = j[args.first]), ...); }, this_t::s_member_tuple);\
	}\
	template < typename tjson >\
	friend void to_json(tjson&& j, this_t const& var) {\
		std::apply([&j, &var](auto const& ... args) { ((j[args.first] = var.*(args.second)), ...); }, this_t::s_member_tuple);\
	}\
	template < typename tarchive >\
	friend tarchive& operator >> (tarchive& ar, this_t& var) {\
		std::apply([&ar, &var](auto& ... args) { ((ar >> var.*(args.second)), ...); }, this_t::s_member_tuple);\
		return ar;\
	}\
	template < typename tarchive >\
	friend tarchive& operator << (tarchive& ar, this_t const& var) {\
		std::apply([&ar, &var](auto const& ... args) { ((ar << var.*(args.second)), ...); }, this_t::s_member_tuple);\
		return ar;\
	}\
	//auto operator <=> (this_t const&) const = default;


#define GTL__REFLECTION_DERIVED()\
	using reflection_base_t = base_t::reflection_base_t;\
	template < typename tjson >\
	friend void from_json(tjson const& j, this_t& var) {\
		from_json(j, (base_t&)var);\
		std::apply([&j, &var](auto& ... args) { ((var.*(args.second) = j[args.first]), ...); }, this_t::s_member_tuple);\
	}\
	template < typename tjson >\
	friend void to_json(tjson&& j, this_t const& var) {\
		to_json(j, (base_t const&)var);\
		std::apply([&j, &var](auto const& ... args) { ((j[args.first] = var.*(args.second)), ...); }, this_t::s_member_tuple);\
	}\
	template < typename tarchive >\
	friend tarchive& operator >> (tarchive& ar, this_t& var) {\
		ar >> (base_t&)var;\
		std::apply([&ar, &var](auto& ... args) { ((ar >> var.*(args.second)), ...); }, this_t::s_member_tuple);\
		return ar;\
	}\
	template < typename tarchive >\
	friend tarchive& operator << (tarchive& ar, this_t const& var) {\
		ar << (base_t const&)var;\
		std::apply([&ar, &var](auto const& ... args) { ((ar << var.*(args.second)), ...); }, this_t::s_member_tuple);\
		return ar;\
	}\
	//auto operator <=> (this_t const&) const = default;


	//-----------------------------------------------------------------------------
	// Reflection (member wise...) : with virtual function
#define GTL__REFLECTION_VIRTUAL_BASE(TJSON)\
	GTL__REFLECTION_BASE(TJSON)\
	virtual void FromJson(json_t const& j) { from_json(j, *(this_t*)this); }\
	virtual void ToJson(json_t& j) const { to_json(j, *(this_t const*)this); }\
	virtual bool Compare(this_t const& B) const {\
		if (!dynamic_cast<this_t const*>(&B))\
			return false;\
		return *this == B;\
	}\


#define GTL__REFLECTION_VIRTUAL_DERIVED()\
	GTL__REFLECTION_DERIVED()\
	void FromJson(typename base_t::json_t const& j) override { from_json(j, *(this_t*)this); }\
	void ToJson(typename base_t::json_t& j) const override { to_json(j, *(this_t const*)this); }\
	bool Compare(typename base_t::reflection_base_t const& B) const override {\
		if (!dynamic_cast<this_t const*>(&B))\
			return false;\
		return (*this) == (this_t const&)B;\
	}\



	////================================================================================================================================
	//// reflection CRTP version.
	////

	///// @brief CRTP reflection class. (not good. better to use MACROS)
	///// @tparam 
	//template < typename THIS_CLASS, typename tjson = gtl::njson<nlohmann::json> >
	//class IReflection {
	//public:
	//	using reflection_base_t = THIS_CLASS;
	//	using this_t = THIS_CLASS;

	//	GTL__REFLECTION_VIRTUAL_BASE(tjson)

	////public:
	////	auto operator <=>(IReflection const&) const = default;
	//};



	///// @brief CRTP reflection class (derived). (not good. better to use MACROS)
	///// @tparam 
	//template < typename THIS_CLASS, typename BASE_CLASS >
	//class IReflectionDerived : public BASE_CLASS {
	//public:
	//	using this_t = THIS_CLASS;
	//	using base_t = BASE_CLASS;

	//	GTL__REFLECTION_VIRTUAL_DERIVED()

	//public:
	//	auto operator <=>(IReflectionDerived const&) const = default;
	//};


#pragma pack(pop)
}	// namespace gtl

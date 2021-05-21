﻿#pragma once


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

	#define GTL_REFL__MEMBER_TABLE\
		constexpr inline static const std::tuple member_tuple_s

	//#define GTL_REFL__MEMBERS(...)\
	//		I_GTL_REFL__MEMBERS_(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)


	#define I_GTL_REFL__MEMBER(var)\
			gtl::internal::pair{ #var##sv, &this_t::var }

	#define I_GTL_REFL__MEMBER_EX(var, expr)		I_GTL_REFL__MEMBER(var), expr

	#define I_GTL_REFL__MEMBER_1(var)				I_GTL_REFL__MEMBER(var)
	#define I_GTL_REFL__MEMBER_2(var1, var2)		I_GTL_REFL__MEMBER(var1), I_GTL_REFL__MEMBER(var2)
	#define I_GTL_REFL__MEMBER_3(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_2(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_4(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_3(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_5(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_4(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_6(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_5(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_7(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_6(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_8(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_7(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_9(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_8(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_10(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_9(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_11(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_10(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_12(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_11(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_13(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_12(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_14(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_13(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_15(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_14(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_16(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_15(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_17(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_16(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_18(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_17(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_19(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_18(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_20(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_19(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_21(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_20(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_22(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_21(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_23(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_22(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_24(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_23(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_25(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_24(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_26(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_25(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_27(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_26(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_28(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_27(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_29(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_28(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_30(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_29(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_31(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_30(__VA_ARGS__))
	#define I_GTL_REFL__MEMBER_32(var, ...)			I_GTL_REFL__MEMBER_EX(var, I_GTL_REFL__MEMBER_31(__VA_ARGS__))

	#define I_GTL_REFL__MEMBERS_C(N, ...)	I_GTL_REFL__MEMBER_##N (__VA_ARGS__)
	#define I_GTL_REFL__MEMBERS_F(N, ...)	I_GTL_REFL__MEMBERS_C(N, __VA_ARGS__)
	#define GTL_REFL__MEMBERS(...)			I_GTL_REFL__MEMBERS_F(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)


	//================================================================================================================================
	// reflection - MACRO version
	//


#define GTL_REFL__CLASS__BASE(THIS_CLASS)\
	using mw_base_t = THIS_CLASS;\
	using this_t = THIS_CLASS;\
	template < typename tjson >\
	friend void from_json(tjson const& j, this_t& var) {\
		std::apply([&j, &var](auto& ... args) { ((var.*(args.second) = j[args.first]), ...); }, this_t::member_tuple_s);\
	}\
	template < typename tjson >\
	friend void to_json(tjson& j, this_t const& var) {\
		std::apply([&j, &var](auto const& ... args) { ((j[args.first] = var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);\
	}\
	template < typename tarchive >\
	friend tarchive& operator >> (tarchive& ar, this_t& var) {\
		std::apply([&ar, &var](auto& ... args) { ((ar >> var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);\
		return ar;\
	}\
	template < typename tarchive >\
	friend tarchive& operator << (tarchive& ar, this_t const& var) {\
		std::apply([&ar, &var](auto const& ... args) { ((ar << var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);\
		return ar;\
	}\
	//auto operator <=> (this_t const&) const = default;


#define GTL_REFL__CLASS__DERIVED(THIS_CLASS, PARENT_CLASS)\
	using this_t = THIS_CLASS;\
	using parent_t = PARENT_CLASS;\
	template < typename tjson >\
	friend void from_json(tjson const& j, this_t& var) {\
		from_json(j, (parent_t&)var);\
		std::apply([&j, &var](auto& ... args) { ((var.*(args.second) = j[args.first]), ...); }, this_t::member_tuple_s);\
	}\
	template < typename tjson >\
	friend void to_json(tjson& j, this_t const& var) {\
		to_json(j, (parent_t const&)var);\
		std::apply([&j, &var](auto const& ... args) { ((j[args.first] = var.*(args.second)), ...); }, this_t::member_tuple_s);\
	}\
	template < typename tarchive >\
	friend tarchive& operator >> (tarchive& ar, this_t& var) {\
		ar >> (parent_t&)var;\
		std::apply([&ar, &var](auto& ... args) { ((ar & var.*(args.second)), ...); }, this_t::member_tuple_s);\
		return ar;\
	}\
	template < typename tarchive >\
	friend tarchive& operator & (tarchive& ar, this_t const& var) {\
		ar << (parent_t const&)var;\
		std::apply([&ar, &var](auto const& ... args) { ((ar & var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);\
		return ar;\
	}\
	//auto operator <=> (this_t const&) const = default;


	//-----------------------------------------------------------------------------
	// Reflection (member wise...) : with virtual function
#define GTL_REFL__CLASS__BASE_VIRTUAL(THIS_CLASS)\
	GTL_REFL__CLASS__BASE(THIS_CLASS)\
	virtual void FromJson(bjson<> const& j) { from_json(j, *(this_t*)this); }\
	virtual void ToJson(bjson<>& j) const { to_json(j, *(this_t*)this); }\
	virtual void FromJson(njson<> const& j) { from_json(j, *(this_t*)this); }\
	virtual void ToJson(njson<>& j) const { to_json(j, *(this_t*)this); }\
	virtual bool Compare(this_t const& B) const {\
		if (!dynamic_cast<this_t const*>(&B))\
			return false;\
		return *this == B;\
	}\


#define GTL_REFL__CLASS__VIRTUAL_DERIVED(THIS_CLASS, PARENT_CLASS)\
	GTL_REFL__CLASS__DERIVED(THIS_CLASS, PARENT_CLASS)\
	void FromJson(bjson<> const& j) override { from_json(j, *(THIS_CLASS*)this); }\
	void ToJson(bjson<>& j) const override { to_json(j, *(THIS_CLASS*)this); }\
	void FromJson(njson<> const& j) override { from_json(j, *(THIS_CLASS*)this); }\
	void ToJson(njson<>& j) const override { to_json(j, *(THIS_CLASS*)this); }\
	bool Compare(typename THIS_CLASS::mw_base_t const& B) const override {\
		if (!dynamic_cast<this_t const*>(&B))\
			return false;\
		return (*this) == (this_t const&)B;\
	}\



	//================================================================================================================================
	// reflection CRTP version.
	//

	/// @brief CRTP reflection class. (not good. better to use MACROS)
	/// @tparam 
	template < typename THIS_CLASS >
	class IMemberWise {
	public:
		using mw_base_t = THIS_CLASS;
		using this_t = THIS_CLASS;

		template < typename tjson >
		friend void from_json(tjson const& j, this_t& var) {
		#if 1
			std::apply([&j, &var](auto& ... args) { ((var.*(args.second) = j[args.first]), ...); }, this_t::member_tuple_s);
		#else
			for...(auto& [name, pVar] : this_t::member_tuple_s) {
				var.*pVar = j[name];
			}
		#endif
		}

		template < typename tjson >
		friend void to_json(tjson& j, this_t const& var) {
			j.clear();

		#if 1
			std::apply([&j, &var](auto const& ... args) { ((j[args.first] = var.*(args.second)), ...); }, this_t::member_tuple_s);
		#else
			for...(auto& [name, pVar] : this_t::member_tuple_s) {
				j[name] = var.*pVar;
			}
		#endif
		}
		template < typename Archive >
		friend Archive& operator & (Archive& ar, this_t& var) {
			std::apply([&var, &ar](auto& ... args) { ((ar & var.*(args.second)), ...); }, this_t::member_tuple_s);
			return ar;
		}
		template < typename Archive >
		friend Archive& operator & (Archive& ar, this_t const& var) {
			std::apply([&var, &ar](auto const& ... args) { ((ar & var.*(args.second)), ...); }, this_t::member_tuple_s);
			return ar;
		}

	//public:
	//	auto operator <=>(IMemberWise const&) const = default;
	};



	/// @brief CRTP reflection class (derived). (not good. better to use MACROS)
	/// @tparam 
	template < typename THIS_CLASS, typename PARENT_CLASS >
	class IMemberWiseDerived : public PARENT_CLASS {
	public:
		using this_t = THIS_CLASS;
		using parent_t = PARENT_CLASS;
		using mw_base_t = typename PARENT_CLASS::mw_base_t;

		// from_json
		template < typename tjson >
		friend void from_json(tjson const& j, this_t& var) {
			from_json(j, (parent_t&)var);
			std::apply([&j, &var](auto& ... args) { ((var.*(args.second) = j[args.first]), ...); }, this_t::member_tuple_s);
		}
		// to_json
		template < typename tjson >
		friend void to_json(tjson& j, this_t const& var) {
			to_json(j, (parent_t const&)var);
			std::apply([&j, &var](auto const& ... args) { ((j[args.first] = var.*(args.second)), ...); }, this_t::member_tuple_s);
		}

		// archive & var
		template < typename tarchive >
		friend tarchive& operator >> (tarchive& ar, this_t& var) {
			ar >> (parent_t&)var;
			std::apply([&var, &ar](auto& ... args) { ((ar << var.*(args.second)), ...); }, this_t::member_tuple_s);
			return ar;
		}
		// archive & var
		template < typename tarchive >
		friend tarchive& operator << (tarchive& ar, this_t const& var) {
			ar << (parent_t const&)var;
			std::apply([&var, &ar](auto const& ... args) { ((ar & var.*(args.second)), ...); }, this_t::member_tuple_s);
			return ar;
		}

	//public:
	//	auto operator <=>(IMemberWiseDerived const&) const = default;
	};



	/// @brief CRTP reflection class. (virtual) (not good. better to use MACROS)
	/// @tparam 
	template < typename THIS_CLASS >
	class IMemberWiseV : public IMemberWise<THIS_CLASS> {
	public:
		virtual void FromJson(bjson<> const& j) { from_json(j, *(THIS_CLASS*)this); }
		virtual void ToJson(bjson<>& j) const   { to_json(j, *(THIS_CLASS*)this); }
		virtual void FromJson(njson<> const& j) { from_json(j, *(THIS_CLASS*)this); }
		virtual void ToJson(njson<>& j) const   { to_json(j, *(THIS_CLASS*)this); }

		virtual bool Compare(THIS_CLASS const& B) const {
			if (!dynamic_cast<THIS_CLASS const*>(&B))
				return false;
			return *this == B;
		}

	//public:
	//	auto operator <=>(IMemberWiseV const&) const = default;
	};



	/// @brief CRTP reflection class (virtual. derived). (not good. better to use MACROS)
	/// @tparam 
	template < typename THIS_CLASS, typename PARENT_CLASS >
	class IMemberWiseDerivedV : public IMemberWiseDerived<PARENT_CLASS, THIS_CLASS> {
	public:
		void FromJson(bjson<> const& j) override { from_json(j, *(THIS_CLASS*)this); }
		void ToJson(bjson<>& j) const override   { to_json(j, *(THIS_CLASS*)this); }
		void FromJson(njson<> const& j) override { from_json(j, *(THIS_CLASS*)this); }
		void ToJson(njson<>& j) const override   { to_json(j, *(THIS_CLASS*)this); }

		bool Compare(typename PARENT_CLASS::mw_base_t const& B) const override {
			if (!dynamic_cast<THIS_CLASS const*>(&B))
				return false;
			return (*this) == (THIS_CLASS const&)B;
		}

	//public:
	//	auto operator <=>(IMemberWiseDerivedV const&) const = default;
	};



#pragma pack(pop)
}	// namespace gtl
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

	// MW : (M)ember (W)ise

	#define MW__BEGIN_TABLE()\
		constexpr inline static const std::tuple member_tuple_s {\


	#define MW__ADD_MEMBER(var)\
			gtl::internal::pair{ #var##sv, &this_t::var },

	#define MW__END_TABLE()\
		};\

	#define MW__MEMBER_EX(var, expr)	MW__ADD_MEMBER(var) expr

	#define MW__MEMBER_1(var)			MW__ADD_MEMBER(var)
	#define MW__MEMBER_2(var1, var2)    MW__ADD_MEMBER(var1) MW__ADD_MEMBER(var2)
	#define MW__MEMBER_3(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_2(__VA_ARGS__))
	#define MW__MEMBER_4(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_3(__VA_ARGS__))
	#define MW__MEMBER_5(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_4(__VA_ARGS__))
	#define MW__MEMBER_6(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_5(__VA_ARGS__))
	#define MW__MEMBER_7(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_6(__VA_ARGS__))
	#define MW__MEMBER_8(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_7(__VA_ARGS__))
	#define MW__MEMBER_9(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_8(__VA_ARGS__))
	#define MW__MEMBER_10(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_9(__VA_ARGS__))
	#define MW__MEMBER_11(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_10(__VA_ARGS__))
	#define MW__MEMBER_12(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_11(__VA_ARGS__))
	#define MW__MEMBER_13(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_12(__VA_ARGS__))
	#define MW__MEMBER_14(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_13(__VA_ARGS__))
	#define MW__MEMBER_15(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_14(__VA_ARGS__))
	#define MW__MEMBER_16(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_15(__VA_ARGS__))
	#define MW__MEMBER_17(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_16(__VA_ARGS__))
	#define MW__MEMBER_18(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_17(__VA_ARGS__))
	#define MW__MEMBER_19(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_18(__VA_ARGS__))
	#define MW__MEMBER_20(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_19(__VA_ARGS__))
	#define MW__MEMBER_21(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_20(__VA_ARGS__))
	#define MW__MEMBER_22(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_21(__VA_ARGS__))
	#define MW__MEMBER_23(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_22(__VA_ARGS__))
	#define MW__MEMBER_24(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_23(__VA_ARGS__))
	#define MW__MEMBER_25(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_24(__VA_ARGS__))
	#define MW__MEMBER_26(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_25(__VA_ARGS__))
	#define MW__MEMBER_27(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_26(__VA_ARGS__))
	#define MW__MEMBER_28(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_27(__VA_ARGS__))
	#define MW__MEMBER_29(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_28(__VA_ARGS__))
	#define MW__MEMBER_30(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_29(__VA_ARGS__))
	#define MW__MEMBER_31(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_30(__VA_ARGS__))
	#define MW__MEMBER_32(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_31(__VA_ARGS__))

	#define MW__MEMBERS_O2(N, ...) MW__MEMBER_##N (__VA_ARGS__)
	#define MW__MEMBERS_O1(N, ...) MW__MEMBERS_O2(N, __VA_ARGS__)
	#define MW__MEMBERS(...)      MW__MEMBERS_O1(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)


	//-----------------------------------------------------------------------------
	// IMemberWise
	// 
#define DECL_MEMBER_WISE_BASE(THIS_CLASS)\
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
	auto operator <=> (this_t const&) const = default;


#define DECL_MEMBER_WISE_DERIVED(THIS_CLASS, PARENT_CLASS)\
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
	auto operator <=> (this_t const&) const = default;


	//-----------------------------------------------------------------------------
	// IMemberWiseV : with virtual function
#define DECL_MEMBER_WISE_BASE_VIRTUAL(THIS_CLASS)\
	DECL_MEMBER_WISE_BASE(THIS_CLASS)\
	virtual void FromJson(bjson<> const& j) { from_json(j, *(this_t*)this); }\
	virtual void ToJson(bjson<>& j) const { to_json(j, *(this_t*)this); }\
	virtual void FromJson(njson<> const& j) { from_json(j, *(this_t*)this); }\
	virtual void ToJson(njson<>& j) const { to_json(j, *(this_t*)this); }\
	virtual bool Compare(this_t const& B) const {\
		if (!dynamic_cast<this_t const*>(&B))\
			return false;\
		return *this == B;\
	}\


#define DECL_MEMBER_WISE_VIRTUAL_DERIVED(THIS_CLASS, PARENT_CLASS)\
	DECL_MEMBER_WISE_DERIVED(THIS_CLASS, PARENT_CLASS)\
	void FromJson(bjson<> const& j) override { from_json(j, *(THIS_CLASS*)this); }\
	void ToJson(bjson<>& j) const override { to_json(j, *(THIS_CLASS*)this); }\
	void FromJson(njson<> const& j) override { from_json(j, *(THIS_CLASS*)this); }\
	void ToJson(njson<>& j) const override { to_json(j, *(THIS_CLASS*)this); }\
	bool Compare(typename THIS_CLASS::mw_base_t const& B) const override {\
		if (!dynamic_cast<this_t const*>(&B))\
			return false;\
		return (*this) == (this_t const&)B;\
	}\




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

	public:
		auto operator <=>(IMemberWise const&) const = default;
	};



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

	public:
		auto operator <=>(IMemberWiseDerived const&) const = default;
	};


	//-----------------------------------------------------------------------------
	// IMemberWiseV : with virtual function
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

	public:
		auto operator <=>(IMemberWiseV const&) const = default;
	};

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

	public:
		auto operator <=>(IMemberWiseDerivedV const&) const = default;
	};


#pragma pack(pop)
}	// namespace gtl

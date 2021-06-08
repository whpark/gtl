#pragma once

//////////////////////////////////////////////////////////////////////
//
// _macro.h: macros to export from gtl_module.
//
// PWH
//    2021.05.20.
//
//////////////////////////////////////////////////////////////////////


#include <version>	// version information
#include <source_location>
#include "gtl/_config.h"

#define GTL_DEPR_SEC [[deprecated("NOT Secure")]]

// until c++20 source_location
#ifdef __cpp_lib_source_location
#	define GTL__FUNCSIG std::string(std::source_location::current().function_name()) + " : "
#else
	#ifdef _MSC_VER
		#define GTL__FUNCSIG __FUNCSIG__ " : "
	#else
		#error ..... ????...
	#endif
#endif

#define NOMINMAX	// disable Windows::min/max
#if defined(min) || defined(max)
//#	error min/max must not be defined. turn it off using '#define NOMINMAX' before '#include <Windows.h>''
#	undef min
#	undef max
#endif


// NUM_ARGS(...) macro arg °¹¼ö
#define NUM_ARGS_64(                            _64,_63,_62,_61,_60,_59,_58,_57,_56,_55,_54,_53,_52,_51,_50,_49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1, N, ...) N
#define NUM_ARGS(...) NUM_ARGS_64( __VA_ARGS__ , 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
static_assert(NUM_ARGS(1, 2, 3) == 3, "for MSVC, add compiler option /Zc:preprocessor");


#define I_GTL__RECURSIVE_MACRO(macro, var)								macro(var)
#define I_GTL__RECURSIVE_MACRO_EX_COMMA(macro, var, expr)				macro(var), expr
#define I_GTL__RECURSIVE_MACRO_EX_SPACE(macro, var, expr)				macro(var)  expr
#define I_GTL__RECURSIVE_MACRO_1(seperator, macro, var)					macro(var)
#define I_GTL__RECURSIVE_MACRO_2(seperator, macro, var, var2)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_1(seperator, macro, var2))
#define I_GTL__RECURSIVE_MACRO_3(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_2(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_4(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_3(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_5(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_4(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_6(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_5(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_7(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_6(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_8(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_7(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_9(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_8(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_10(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_9(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_11(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_10(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_12(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_11(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_13(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_12(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_14(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_13(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_15(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_14(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_16(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_15(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_17(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_16(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_18(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_17(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_19(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_18(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_20(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_19(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_21(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_20(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_22(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_21(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_23(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_22(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_24(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_23(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_25(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_24(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_26(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_25(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_27(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_26(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_28(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_27(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_29(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_28(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_30(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_29(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_31(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_30(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_32(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_31(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_33(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_32(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_34(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_33(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_35(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_34(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_36(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_35(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_37(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_36(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_38(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_37(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_39(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_38(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_40(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_39(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_41(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_40(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_42(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_41(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_43(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_42(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_44(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_43(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_45(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_44(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_46(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_45(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_47(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_46(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_48(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_47(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_49(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_48(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_50(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_49(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_51(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_50(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_52(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_51(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_53(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_52(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_54(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_53(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_55(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_54(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_56(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_55(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_57(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_56(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_58(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_57(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_59(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_58(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_60(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_59(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_61(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_60(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_62(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_61(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_63(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_62(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_64(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_63(seperator, macro, __VA_ARGS__))

#define I_GTL__RECURSIVE_MACRO_C(seperator, macro, N, ...)				I_GTL__RECURSIVE_MACRO_##N(seperator, macro, __VA_ARGS__)
#define I_GTL__RECURSIVE_MACRO_F(seperator, macro, N, ...)				I_GTL__RECURSIVE_MACRO_C(seperator, macro, N, __VA_ARGS__)
#define GTL__RECURSIVE_MACRO_COMMA(macro, ...)							I_GTL__RECURSIVE_MACRO_F(COMMA, macro, NUM_ARGS(__VA_ARGS__), __VA_ARGS__)
#define GTL__RECURSIVE_MACRO_SPACE(macro, ...)							I_GTL__RECURSIVE_MACRO_F(SPACE, macro, NUM_ARGS(__VA_ARGS__), __VA_ARGS__)


//-----------------------------------------------------------------------------
// Text Constant macros
#define TEXT_A(x) x
#define ___WIDE_TEXT(x) L##x
#define TEXT_W(x) ___WIDE_TEXT(x)
#define ___UTF8_TEXT(x) u8##x
#define TEXT_u8(x) ___UTF8_TEXT(x)
#define ___UTF16_TEXT(x) u##x
#define TEXT_u(x) ___UTF16_TEXT(x)
#define ___UTF32_TEXT(x) U##x
#define TEXT_U(x) ___UTF32_TEXT(x)

#define _EOL			"\r\n"
#define EOL				TEXT_A(_EOL)
#define EOLA			TEXT_A(_EOL)
#define EOLW			TEXT_W(_EOL)
#define EOLu8			TEXT_u8(_EOL)
#define EOLu			TEXT_u(_EOL)
#define EOLU			TEXT_U(_EOL)
#define SPACE_STRING	" \t\r\n"



//-----------------------------------------------------------------------------
// dynamic
#define GTL__DYNAMIC_VIRTUAL_INTERFACE(className)\
	using mw_base_t = className;\
	using mw_this_t = className;\
	virtual std::unique_ptr<mw_base_t> NewObject() const = 0;\
	virtual std::unique_ptr<mw_base_t> NewClone()  const = 0;\
	friend mw_base_t* new_clone(mw_this_t const& r) { return r.NewClone().release(); }

#define GTL__DYNAMIC_VIRTUAL_BASE(className)\
	using mw_base_t = className;\
	using mw_this_t = className;\
	virtual std::unique_ptr<mw_base_t> NewObject() const { return std::make_unique<mw_this_t>(); }\
	virtual std::unique_ptr<mw_base_t> NewClone()  const { return std::make_unique<mw_this_t>(*this); }\
	friend mw_this_t* new_clone(mw_this_t const& r) { return (mw_this_t*)r.NewClone().release(); }

#define GTL__DYNAMIC_VIRTUAL_DERIVED(className)\
	using mw_this_t = className;\
	virtual std::unique_ptr<mw_base_t> NewObject() const override { return std::make_unique<mw_this_t>(); }\
	virtual std::unique_ptr<mw_base_t> NewClone()  const override { return std::make_unique<mw_this_t>(*this); }\
	friend mw_this_t* new_clone(mw_this_t const& r) { return (mw_this_t*)r.NewClone().release(); }

#define GTL__DYNAMIC_BASE(T_IDENTIFIER)\
	static inline TDynamicCreateBase<this_t, T_IDENTIFIER> dynamicCreateBase_s;

//#define GTL__DYNAMIC_CLASS(ID)\
//	static inline TDynamicCreateHelper<this_t, ID, []() -> std::unique_ptr<mw_base_t>{ return std::make_unique<this_t>(); }> dynamicCreateDerived_s;
#define GTL__DYNAMIC_CLASS(ID)\
	static inline TDynamicCreateHelper<this_t, ID, std::make_unique<this_t>> dynamicCreateDerived_s;
#define GTL__DYNAMIC_CLASS_EMPLACE(ID, ...)\
	static inline TDynamicCreateHelper<this_t, ID, []()->std::unique_ptr<mw_base_t>{ return std::make_unique<this_t>(__VA_ARGS__); }> dynamicCreateDerived_s;


//================================================================================================================================
// reflection
//
#define I_GTL__REFLECTION_MEMBER(var)	gtl::internal::pair{ std::string_view{#var}, &this_t::var }

#define GTL__REFLECTION_MEMBERS(...) \
	constexpr inline static const std::tuple member_tuple_s {\
		GTL__RECURSIVE_MACRO_COMMA(I_GTL__REFLECTION_MEMBER, __VA_ARGS__)\
	};

//================================================================================================================================
// reflection - MACRO version
//


#define GTL__REFLECTION_BASE(THIS_CLASS)\
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


#define GTL__REFLECTION_DERIVED(THIS_CLASS, PARENT_CLASS)\
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
#define GTL__REFLECTION_VIRTUAL_BASE(THIS_CLASS)\
	GTL__REFLECTION_BASE(THIS_CLASS)\
	virtual void FromJson(bjson<> const& j) { from_json(j, *(this_t*)this); }\
	virtual void ToJson(bjson<>& j) const { to_json(j, *(this_t*)this); }\
	virtual void FromJson(njson<> const& j) { from_json(j, *(this_t*)this); }\
	virtual void ToJson(njson<>& j) const { to_json(j, *(this_t*)this); }\
	virtual bool Compare(this_t const& B) const {\
		if (!dynamic_cast<this_t const*>(&B))\
			return false;\
		return *this == B;\
	}\


#define GTL__REFLECTION_VIRTUAL_DERIVED(THIS_CLASS, PARENT_CLASS)\
	GTL__REFLECTION_DERIVED(THIS_CLASS, PARENT_CLASS)\
	void FromJson(bjson<> const& j) override { from_json(j, *(THIS_CLASS*)this); }\
	void ToJson(bjson<>& j) const override { to_json(j, *(THIS_CLASS*)this); }\
	void FromJson(njson<> const& j) override { from_json(j, *(THIS_CLASS*)this); }\
	void ToJson(njson<>& j) const override { to_json(j, *(THIS_CLASS*)this); }\
	bool Compare(typename THIS_CLASS::mw_base_t const& B) const override {\
		if (!dynamic_cast<this_t const*>(&B))\
			return false;\
		return (*this) == (this_t const&)B;\
	}\




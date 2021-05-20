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
#	error min/max must not be defined. turn it off using '#define NOMINMAX' before '#include <Windows.h>''
#endif


// NUM_ARGS(...) macro arg °¹¼ö
#define NUM_ARGS_64(                            _64,_63,_62,_61,_60,_59,_58,_57,_56,_55,_54,_53,_52,_51,_50,_49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1, N, ...) N
#define NUM_ARGS(...) NUM_ARGS_64( __VA_ARGS__ , 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
static_assert(NUM_ARGS(1, 2, 3) == 3, "for MSVC, add compiler option /Zc:preprocessor");


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
#define GTL__VIRTUAL_DYNAMIC_INTERFACE\
	using mw_base_t = this_t;\
	virtual std::unique_ptr<mw_base_t> NewObject() = 0;\
	virtual std::unique_ptr<mw_base_t> NewClone() = 0;

#define GTL__VIRTUAL_DYNAMIC_BASE\
	virtual std::unique_ptr<mw_base_t> NewObject() { return std::make_unique<this_t>(); }\
	virtual std::unique_ptr<mw_base_t> NewClone()  { return std::make_unique<this_t>(*this); }

#define GTL__VIRTUAL_DYNAMIC_DERIVED\
	virtual std::unique_ptr<mw_base_t> NewObject() override { return std::make_unique<this_t>(); }\
	virtual std::unique_ptr<mw_base_t> NewClone()  override { return std::make_unique<this_t>(*this); }

#define GTL_DYN__BASE(T_IDENTIFIER)\
	static inline TDynamicCreateBase<this_t, T_IDENTIFIER> dynamicCreateBase_s;

//#define GTL_DYN__CLASS(ID)\
//	static inline TDynamicCreateHelper<this_t, ID, []() -> std::unique_ptr<mw_base_t>{ return std::make_unique<this_t>(); }> dynamicCreateDerived_s;
#define GTL_DYN__CLASS(ID)\
	static inline TDynamicCreateHelper<this_t, ID, std::make_unique<this_t>> dynamicCreateDerived_s;
#define GTL_DYN__CLASS_EMPLACE(ID, ...)\
	static inline TDynamicCreateHelper<this_t, ID, []()->std::unique_ptr<mw_base_t>{ return std::make_unique<this_t>(__VA_ARGS__); }> dynamicCreateDerived_s;


	//================================================================================================================================
	// reflection
	//
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




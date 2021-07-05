#include "pch.h"

//#include "gtl/reflection.h"
//#include "gtl/dynamic.h"

import gtl;

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::reflection::MACRO {

	struct CTestStruct {

	public:
		std::string str1{"str1"};
		std::string str2{"str2"};

	private:
		GTL__REFLECTION_BASE(CTestStruct)

	#if 1
		GTL__REFLECTION_MEMBERS(str1,str2)
	#else
		constexpr static inline const std::tuple member_tuple_s{
			gtl::internal::pair{ "str1"sv, &this_t::str1 },
			gtl::internal::pair{ "str2"sv, &this_t::str2 },
		};
	#endif

	};


	class CTestClass {

	public:
		bool b1{}, b2{};
		int i{}, j{};
		int64_t k{}, l{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };
		CTestStruct test;

	private:
		GTL__REFLECTION_BASE(CTestClass)
		
		GTL__REFLECTION_MEMBERS(b1, b2,
								i, j,
								k, l,
								a, b, c,
								str, strU8,
								test);

	};


	class CTestClassDerived : public CTestClass {

	public:
		int k{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };

	private:
		GTL__REFLECTION_DERIVED(CTestClassDerived, CTestStruct)

		GTL__REFLECTION_MEMBERS(k, a, b, c, str, strU8);
	};

}


namespace gtl::test::reflection::CRTP {

	struct CTestStruct : public IMemberWise<CTestStruct> {
	public:
		std::string str1{"str1"};
		std::string str2{"str2"};

	public:
		//using this_t = std::remove_cvref_t<decltype(*this)>;

		GTL__REFLECTION_MEMBERS(str1, str2);

	};


	class CTestClass : public IMemberWise<CTestClass> {
	public:
		bool b1{}, b2{};
		int i{}, j{};
		int64_t k{}, l{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };
		CTestStruct test;

		GTL__REFLECTION_MEMBERS(b1,
								b2,
								i, j,
								k, l,
								a, b, c,
								str, strU8,
								test);
	};


	class CTestClassDerived : public IMemberWiseDerived<CTestClassDerived, CTestClass> {
	public:
		int k{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };

	private:
		GTL__REFLECTION_MEMBERS(k, a, b, c, str, strU8);
	};

}

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
		GTL_REFL__CLASS__BASE(CTestStruct)

	#if 1
		GTL_REFL__MEMBER_TABLE {
			GTL_REFL__MEMBERS(str1),
			GTL_REFL__MEMBERS(str2)
		};
	#else
		constexpr inline static const std::tuple member_tuple_s{
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
		GTL_REFL__CLASS__BASE(CTestClass)
		
		GTL_REFL__MEMBER_TABLE {
			GTL_REFL__MEMBERS(b1),
			GTL_REFL__MEMBERS(b2),
			GTL_REFL__MEMBERS(i, j),
			GTL_REFL__MEMBERS(k, l),
			GTL_REFL__MEMBERS(a, b, c),
			GTL_REFL__MEMBERS(str, strU8),
			GTL_REFL__MEMBERS(test)
		};

	};


	class CTestClassDerived : public CTestClass {

	public:
		int k{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };

	private:
		GTL_REFL__CLASS__DERIVED(CTestClassDerived, CTestStruct)

		GTL_REFL__MEMBER_TABLE {
			GTL_REFL__MEMBERS(k, a, b, c, str, strU8)
		};
	};

}


namespace gtl::test::reflection::CRTP {

	struct CTestStruct : public IMemberWise<CTestStruct> {
	public:
		std::string str1{"str1"};
		std::string str2{"str2"};

	public:
		//using this_t = std::remove_cvref_t<decltype(*this)>;

		GTL_REFL__MEMBER_TABLE {
			GTL_REFL__MEMBERS(str1, str2)
		};

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

		GTL_REFL__MEMBER_TABLE {
			GTL_REFL__MEMBERS(b1),
			GTL_REFL__MEMBERS(b2),
			GTL_REFL__MEMBERS(i, j),
			GTL_REFL__MEMBERS(k, l),
			GTL_REFL__MEMBERS(a, b, c),
			GTL_REFL__MEMBERS(str, strU8),
			GTL_REFL__MEMBERS(test),
		};
	};


	class CTestClassDerived : public IMemberWiseDerived<CTestClassDerived, CTestClass> {
	public:
		int k{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };

	private:
		GTL_REFL__MEMBER_TABLE{
			GTL_REFL__MEMBERS(k, a, b, c, str, strU8)
		};
	};

}

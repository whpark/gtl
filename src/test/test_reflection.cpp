#include "pch.h"

#include "gtl/reflection.h"


#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::reflection1 {

	struct CTestStruct : public IMemberWise<CTestStruct> {
	public:
		std::string str1{"str1"};
		std::string str2{"str2"};

	public:
		//using this_t = std::remove_cvref_t<decltype(*this)>;

		MW__BEGIN_TABLE()
			MW__ADD_MEMBER(str1)
			MW__ADD_MEMBER(str2)
		MW__END_TABLE()


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

		MW__BEGIN_TABLE()
			MW__ADD_MEMBER(b1)
			MW__ADD_MEMBER(b2)
			MW__MEMBERS(i, j)
			MW__MEMBERS(k, l)
			MW__MEMBERS(a, b, c)
			MW__MEMBERS(str, strU8)
			MW__MEMBERS(test)
		MW__END_TABLE()

	};


	class CTestClassDerived : public IMemberWiseDerived<CTestClassDerived, CTestClass> {
	public:
		int k{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };

	private:
		MW__BEGIN_TABLE()
			MW__MEMBERS(k, a, b, c, str, strU8)
		MW__END_TABLE()
	};

}


namespace gtl::test::reflection2 {
	struct CTestStruct {
		DECL_MEMBER_WISE_BASE(CTestStruct)

	public:
		std::string str1{"str1"};
		std::string str2{"str2"};

	private:
		MW__BEGIN_TABLE()
			MW__ADD_MEMBER(str1)
			MW__ADD_MEMBER(str2)
		MW__END_TABLE()


	};


	class CTestClass {
		DECL_MEMBER_WISE_BASE(CTestClass)

	public:
		bool b1{}, b2{};
		int i{}, j{};
		int64_t k{}, l{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };
		CTestStruct test;

	private:
		MW__BEGIN_TABLE()
			MW__ADD_MEMBER(b1)
			MW__ADD_MEMBER(b2)
			MW__MEMBERS(i, j)
			MW__MEMBERS(k, l)
			MW__MEMBERS(a, b, c)
			MW__MEMBERS(str, strU8)
			MW__MEMBERS(test)
		MW__END_TABLE()

	};


	class CTestClassDerived : public CTestClass {
		DECL_MEMBER_WISE_DERIVED(CTestClassDerived, CTestStruct)

	public:
		int k{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };

	private:
		MW__BEGIN_TABLE()
			MW__MEMBERS(k, a, b, c, str, strU8)
		MW__END_TABLE()
	};


}

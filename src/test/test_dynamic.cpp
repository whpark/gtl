#include "pch.h"

#include "gtl/reflection.h"
#include "gtl/dynamic.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::reflection::MACRO {

	struct CTestStruct {

	public:
		std::string str1{"str1"};
		std::string str2{"str2"};

		GTL_REFL__CLASS__BASE(CTestStruct)
	private:
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

	public:
		GTL_DYN__BASE(int);
		GTL_DYN__CLASS(0);

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

		virtual ~CTestClass() {}

		GTL_REFL__CLASS__BASE(CTestClass)
	private:
		
		GTL_REFL__MEMBER_TABLE {
			GTL_REFL__MEMBERS(b1),
			GTL_REFL__MEMBERS(b2),
			GTL_REFL__MEMBERS(i, j),
			GTL_REFL__MEMBERS(k, l),
			GTL_REFL__MEMBERS(a, b, c),
			GTL_REFL__MEMBERS(str, strU8),
			GTL_REFL__MEMBERS(test)
		};

	public:
		GTL_DYN__BASE(int);
		GTL_DYN__CLASS(0);
	};


	class CTestClassDerived : public CTestClass {

	public:
		int k{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };

		virtual ~CTestClassDerived() {}

		GTL_REFL__CLASS__DERIVED(CTestClassDerived, CTestStruct)
	private:

		GTL_REFL__MEMBER_TABLE {
			GTL_REFL__MEMBERS(k, a, b, c, str, strU8)
		};

	public:
		//GTL_DYN__CLASS(1);
		constexpr static inline auto Creator = []() -> std::unique_ptr<mw_base_t>{ return std::make_unique<this_t>(); };
		static inline TDynamicCreateHelper<this_t, 1, Creator> dynamicCreateDerived_s;
	};


	class CTestClassDerived2 : public CTestClass {
	public:
		double dummy{};

	public:
		CTestClassDerived2(double d) : dummy(d) {}

		GTL_REFL__CLASS__DERIVED(CTestClassDerived2, CTestStruct)

	private:
		GTL_REFL__MEMBER_TABLE{
			GTL_REFL__MEMBERS(dummy)
		};

	public:
		GTL_DYN__CLASS_EMPLACE(3, 1.2);

	};

	
	
	TEST(dynamic, create) {

		auto p = CTestClass::dynamicCreateBase_s.CreateObject(3);
		auto* p2 = dynamic_cast<CTestClassDerived2*>(p.get());

		EXPECT_TRUE(p2 != nullptr);
		EXPECT_EQ(p2->dummy, 1.2);

	}
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

//namespace boost {
//	// When exceptions are disabled
//	// in standalone, you must provide
//	// this function.
//	BOOST_NORETURN void throw_exception(std::exception const&);
//}

//#include <boost/exception/exception.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <iostream>
#include <sstream>

std::stringstream ss;

class animal
{
public:
	animal() = default;
	animal(int legs) : legs_{legs} {}
	virtual int legs() const { return legs_; }
	virtual ~animal() = default;

private:
	friend class boost::serialization::access;

	template <typename Archive>
	void serialize(Archive &ar, const unsigned int version) { ar & legs_; }

	int legs_;
};

class bird : public animal
{
public:
	bird() = default;
	bird(int legs, bool can_fly) :
		animal{legs}, can_fly_{can_fly} {}
	bool can_fly() const { return can_fly_; }

private:
	friend class boost::serialization::access;

	template <typename Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<animal>(*this);
		ar & can_fly_;
	}

	bool can_fly_;
};

class dog : public animal
{
public:
	dog() = default;
	dog(int legs, bool eat_grass) :
		animal{legs}, eat_grass_{eat_grass} {}
	bool eat_grass() const { return eat_grass_; }

private:
	friend class boost::serialization::access;

	template <typename Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<animal>(*this);
		ar & eat_grass_;
	}

	bool eat_grass_;
};

void save()
{
	boost::archive::text_oarchive oa{ss};
	oa.register_type<bird>();
	oa.register_type<dog>();
	std::unique_ptr<animal> a = std::make_unique<bird>(2, false);
	std::unique_ptr<animal> b = std::make_unique<dog>(4, true);
	oa << a.get();
	oa << b.get();
}

void load()
{
	boost::archive::text_iarchive ia{ss};
	ia.register_type<bird>();
	ia.register_type<dog>();
	std::unique_ptr<animal> a;
	std::unique_ptr<animal> b;
	animal* p1{};
	animal* p2{};
	ia >> p1;
	ia >> p2;
	a.reset(p1);
	b.reset(p2);
	fmt::print("a : {}, b: {}\n", a->legs(), b->legs());
	//std::cout << a->legs() << b->legs() << '\n';
}

TEST(boost, polymorphic_serializing) {
	save();
	load();
}

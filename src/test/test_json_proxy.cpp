#include "pch.h"

#include "nlohmann/json.hpp"
#include "boost/json.hpp"

//#include "gtl/gtl.h"
//#include "gtl/concepts.h"
//#include "gtl/json_proxy.h"
#include "gtl/gtl.h"
import gtl;

#if (GTL__BOOST_JSON__AS_STANDALONE)
	#include "boost/json/src.hpp"   // impl
	//namespace boost {
	//	void throw_exception(std::exception const&) {
	//	}
	//}
#endif

using namespace std::literals;

namespace gtl {


	struct CTestStruct {
	public:
		std::string str1{"str1"};
		std::string str2{"str2"};

	public:
		//using this_t = std::remove_cvref_t<decltype(*this)>;
		using this_t = CTestStruct;

		auto operator <=> (CTestStruct const&) const = default;

		constexpr static inline std::tuple const member_s {
			gtl::internal::pair { "str1"sv, &this_t::str1},
			gtl::internal::pair { "str2"sv, &this_t::str2},
		};

		template < typename tjson >
		friend void from_json(tjson const& j, this_t& object) {
			std::apply([&j, &object](auto& ... pairs) { ((object.*(pairs.second) = j[pairs.first]), ...); }, this_t::member_s); 
		}
		template < typename tjson >
		friend void to_json(tjson& j, this_t const& object) {
			std::apply([&j, &object](auto& ... pairs) { ((j[pairs.first] = object.*(pairs.second)), ...);}, this_t::member_s); 
		}
	};


	class CTestClass {
	public:
		bool b1{}, b2{};
		int i{}, j{};
		int64_t k{}, l{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };
		std::wstring strW { L"strW" };
		CTestStruct test;


	private:
		//using this_t = std::remove_cvref_t<decltype(*this)>;
		using this_t = CTestClass;

		constexpr static inline std::tuple const member_s {
			gtl::internal::pair { "b1"sv, &this_t::b1},
			gtl::internal::pair { "b2"sv, &this_t::b2},
			gtl::internal::pair { "i"sv, &this_t::i},
			gtl::internal::pair { "j"sv, &this_t::j},
			gtl::internal::pair { "k"sv, &this_t::k},
			gtl::internal::pair { "l"sv, &this_t::l},
			gtl::internal::pair { "a"sv, &this_t::a},
			gtl::internal::pair { "b"sv, &this_t::b},
			gtl::internal::pair { "c"sv, &this_t::c},
			gtl::internal::pair { "str"sv, &this_t::str},
			gtl::internal::pair { "strU8"sv, &this_t::strU8},
			gtl::internal::pair { "strW"sv, &this_t::strW},
			gtl::internal::pair { "test"sv, &this_t::test},
		};

	public:
		auto operator <=> (CTestClass const&) const = default;


		// from/to json
		template < typename tjson >
		friend void from_json(tjson const& j, this_t& object) {
			std::apply([&j, &object](auto& ... pairs) { ((object.*(pairs.second) = j[pairs.first]), ...); }, this_t::member_s);
		}
		template < typename tjson >
		friend void to_json(tjson& j, this_t const& object) {
			std::apply([&j, &object](auto& ... pairs) { ((j[pairs.first] = object.*(pairs.second)), ...);}, this_t::member_s);
		}

	};


	class CTestClassDerived : public CTestClass {
	public:
		int k{};
		double a{}, b{}, c{};
		std::string str {"str"};
		std::u8string strU8 { u8"strU8" };

	public:
		//using this_t = std::remove_cvref_t<decltype(*this)>;
		using this_t = CTestClassDerived;
		using base_t = CTestClass;

		auto operator <=> (CTestClassDerived const&) const = default;

		constexpr static inline std::tuple const member_s {
			gtl::internal::pair { "k"sv, &this_t::k},
			gtl::internal::pair { "a"sv, &this_t::a},
			gtl::internal::pair { "b"sv, &this_t::b},
			gtl::internal::pair { "c"sv, &this_t::c},
			gtl::internal::pair { "str"sv, &this_t::str},
			gtl::internal::pair { "strU8"sv, &this_t::strU8},
		};


	public:
		// from/to json

		template < typename tjson > //requires (std::is_base_of_v<base_t, this_t>)//requires (tjson j, this_t obj){ from_json(j, obj); }
		friend void from_json(tjson const& j, this_t& object) {
			tjson jb = j["base_t"sv];
			from_json(jb, (base_t&)object);
			std::apply([&j, &object](auto& ... pairs) { ((object.*(pairs.second) = j[pairs.first]), ...); }, this_t::member_s);
		}
		template < typename tjson > //requires requires (this_t obj) { obj.base_t; }
		friend void to_json(tjson& j, this_t const& object) {
			tjson jb = j["base_t"sv];
			to_json(jb, (base_t const&)object);
			std::apply([&j, &object](auto& ... pairs) { ((j[pairs.first] = object.*(pairs.second)), ...);}, this_t::member_s);
		}

	};

}


TEST(json_proxy, basic) {

	{	// boost::json
		gtl::bjson j;
		gtl::CTestClassDerived a { .k = 3, .a = 3.1415, .c = 333.3, .str = "asdfjksdf", .strU8 = u8"가나다라마"};
		j = a;

		std::cout << '\n' << std::setw(4) << j.json() << '\n';

		gtl::CTestClassDerived b;
		b = j;

		EXPECT_TRUE(a == b);
	}

	{
		// nlohmann::ordered_json
		gtl::njson<nlohmann::ordered_json> j;

		gtl::CTestClassDerived a { .k = 3, .a = 3.1415, .c = 333.3, .str = "asdfjksdf", .strU8 = u8"가나다라마"};
		j = a;

		std::cout << '\n' << std::setw(4) << j.json() << '\n';

		gtl::CTestClassDerived b;
		b = j;

		EXPECT_TRUE(a == b);
	}

	{
		// nlohmann::json
		gtl::njson j;

		gtl::CTestClassDerived a { .k = 3, .a = 3.1415, .c = 333.3, .str = "asdfjksdf", .strU8 = u8"가나다라마"};
		j = a;

		std::cout << '\n' << std::setw(4) << j.json() << '\n';

		gtl::CTestClassDerived b;
		b = j;

		EXPECT_TRUE(a == b);
	}

}

#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/concepts.h"
#include "gtl/json_proxy.h"

using namespace std::literals;

namespace gtl {

	template<typename, typename = void>
	constexpr bool is_type_complete_v = false;

	template<typename T>
	constexpr bool is_type_complete_v<T, std::void_t<decltype(sizeof(T))>> = true;


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

#if 1
		template < typename tjson >
		friend void from_json(tjson const& j, this_t& object) {
			std::apply([&j, &object](auto& ... pairs) { ((object.*(pairs.second) = j[pairs.first]), ...); }, this_t::member_s); 
		}
		template < typename tjson >
		friend void to_json(tjson& j, this_t const& object) {
			std::apply([&j, &object](auto& ... pairs) { ((j[pairs.first] = object.*(pairs.second)), ...);}, this_t::member_s); 
		}
#else
		friend void from_json(bjson const& j, this_t& object) {
			std::apply([&j, &object](auto& ... pairs) { ((object.*(pairs.second) = j[pairs.first]), ...); }, this_t::member_s); 
		}
		friend void to_json(bjson& j, this_t const& object) {
			std::apply([&j, &object](auto& ... pairs) { ((j[pairs.first] = object.*(pairs.second)), ...);}, this_t::member_s); 
		}
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
			gtl::internal::pair { "test"sv, &this_t::test},
		};

	public:
		auto operator <=> (CTestClass const&) const = default;


		// from/to json
#if 1
		template < typename tjson >
		friend void from_json(tjson const& j, this_t& object) {
			//std::apply([&j, &object](auto& ... pairs) { ((object.*(pairs.second) = j[pairs.first]), ...); }, this_t::member_s);
			{ auto const& pairs = std::get< 0>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
			{ auto const& pairs = std::get< 1>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
			{ auto const& pairs = std::get< 2>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
			{ auto const& pairs = std::get< 3>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
			{ auto const& pairs = std::get< 4>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
			{ auto const& pairs = std::get< 5>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
			{ auto const& pairs = std::get< 6>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
			{ auto const& pairs = std::get< 7>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
			{ auto const& pairs = std::get< 8>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
			{ auto const& pairs = std::get< 9>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
			{ auto const& pairs = std::get<10>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
			{ auto const& pairs = std::get<11>(this_t::member_s); object.*(pairs.second) = j[pairs.first]; }
		}
		template < typename tjson >
		friend void to_json(tjson& j, this_t const& object) {
			std::apply([&j, &object](auto& ... pairs) { ((j[pairs.first] = object.*(pairs.second)), ...);}, this_t::member_s);
		}
#else
		friend void from_json(bjson const& j, this_t& object) {
			std::apply([&j, &object](auto& ... pairs) { ((object.*(pairs.second) = j[pairs.first]), ...); }, this_t::member_s);
		}
		friend void to_json(bjson& j, this_t const& object) {
			std::apply([&j, &object](auto& ... pairs) { ((j[pairs.first] = object.*(pairs.second)), ...);}, this_t::member_s);
		}
#endif
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
#if 1
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
#else
		friend void from_json(bjson const& j, this_t& object) {
			from_json(j, (base_t&)object);
			std::apply([&j, &object](auto& ... pairs) { ((object.*(pairs.second) = j[pairs.first]), ...); }, this_t::member_s);
		}
		friend void to_json(bjson& j, this_t const& object) {
			to_json(j, (base_t const&)object);
			std::apply([&j, &object](auto& ... pairs) { ((j[pairs.first] = object.*(pairs.second)), ...);}, this_t::member_s);
		}
#endif
	};

}


void boost_json_tester() {
	{
		using namespace boost::json;
		boost::json::object j;

		j[ "pi" ] = 3.141;                                            // insert a double
		j[ "happy" ] = true;                                          // insert a bool
		j[ "name" ] = "Boost";                                        // insert a string
		j[ "nothing" ] = nullptr;                                     // insert a null
		j[ "answer" ].emplace_object()["everything"] = 42;            // insert an object with 1 element
		j[ "list" ] = { 1, 0, 2 };                                    // insert an array with 3 elements
		j[ "object" ] = { {"currency", "USD"}, {"value", 42.99} };    // insert an object with 2 elements
	}


	{
		//boost::json::value v;
		//gtl::bjson j(v);
		gtl::bjson j;
		gtl::CTestClassDerived a { .k = 3, .a = 3.1415, .c = 333.3, .str = "asdfjksdf", .strU8 = u8"가나다라마"};
		j = a;

		std::cout << '\n' << j.json() << '\n';

		gtl::CTestClassDerived b;
		b = j;

		assert(a == b);
	}

	{
		gtl::njson j;

		gtl::CTestClassDerived a { .k = 3, .a = 3.1415, .c = 333.3, .str = "asdfjksdf", .strU8 = u8"가나다라마"};
		j = a;

		std::cout << '\n' << j.json() << '\n';

		gtl::CTestClassDerived b;
		b = j;

		assert(a == b);
	}

}

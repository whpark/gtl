#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/container_map.h"
#include "gtl/lazy_profile.h"

#include "fmt/compile.h"
#include "boost/lambda/lambda.hpp"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

static_assert(std::is_convertible_v<std::unique_ptr<int>, std::shared_ptr<int>>);
static_assert(!std::is_convertible_v<std::shared_ptr<int>, std::unique_ptr<int>>);
static_assert(!std::is_same_v<std::unique_ptr<int>, std::shared_ptr<int>>);

namespace gtl::test::bll {

	TEST(misc, bll) {

		using namespace boost::lambda;
		std::vector lst{1, 2, 3, 4, 5, 6, 7, 8, 9};
		auto r = lst | std::ranges::views::filter(_1 % 2 == 0);
		fmt::println("{}", r);
		std::for_each(lst.begin(), lst.end(), _1 = 2);
		fmt::println("{}", lst);

	}

}

namespace gtl::test::misc {
	TEST(misc, TLazyProfile) {
		auto sv3 = xStringLiteral{"asdfsdff"};
		auto sv = TStringLiteral<wchar_t, "asdf">().value;
		static auto sv4 = TStringLiteral<wchar_t, "{}">{};
		auto wstr = FormatToTString<wchar_t, "{}">(L"abcd");
		EXPECT_EQ(wstr, L"abcd"s);
		auto str = FormatToTString<char, "{}">("abcdef");
		EXPECT_EQ(str, "abcdef"s);
		auto item = L"  asdf = 1234  ; asdfasdfasdfasdfasdfasdfasdf";
		auto [all, key, value, comment] = ctre::match<R"xxx(\s*(.*[^\s]+)\s*=\s*([^;\n]*|"[^\n]*")\s*((?:;|#).*)?)xxx">(item);
		bool ok = (bool)all;
		std::wstring_view sv2 = key;

		std::string strLazyProfile =
R"xxx(; comment 1
; comment 2
# comment 3

[Test]
abcdef                   = 1                            ; Comments "" 3290 [] asdjfklasd "" \" 23r2390
path                     = 1                            ; Comments "" 3290 [] asdjfklasd "" \" 23r2390
LogTranslatorEvents      = TRUE                         ; COMMENTS asdfas djfkl;asd jfkla;s dfjkla;sdf
;String                   = "\r\n skld\" fjaskdld;f"  garbage here ; comment3


[ key having punctuation()- and spaces ]
; =============
; comments
; =============
ABC                      = 0.1234e4                     ; 1234
   BooleanVar            = TRUE                         ; 
   BooleanVar2           = false                        ; 
   BooleanVar3           = true                         ; 
; another comments
doubleVar                = 3.1415926535897932384626433832795028 ; pi

   path                  = "C:\ABC\DEF\가나다라.txt"       ; path

[ another values ]
    list                 = 1, 2, 3, 4, 5, 6, 7, 8, 9     ; treat as string

)xxx";
		{
			TLazyProfile<char8_t> profile;
			auto r = std::filesystem::current_path();
			std::istringstream stream(strLazyProfile);
			auto t0 = std::chrono::steady_clock::now();
			profile.Load(stream);
			auto t1 = std::chrono::steady_clock::now();
			auto v = profile[u8"Test"](u8"LogTranslatorEvents", false);
			EXPECT_EQ(v, true);
			profile[u8"Test"].SetItemValue(u8"path", 2);
			profile[u8"Test"].SetItemValue(u8"LogTranslatorEvents", false);
			//profile[u8"Test"].SetItemValue(u8"String", u8"\" 바바바바바");	// error.
			profile[u8"Test"].SetItemValue(u8"newValue", 3.1415, u8"comment ___ new");

			profile[u8"key having punctuation()- and spaces"].SetItemValue(u8"path", u8"C:\\ABC\\DEF\\가나다라.txt");

			auto t2 = std::chrono::steady_clock::now();

			profile.Save("ini/lazy_profileU8.ini");

			auto t3 = std::chrono::steady_clock::now();

			auto GetDuration = [](auto t0, auto t1) {
				return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
			};
			fmt::print("Load profile : {}\n", GetDuration(t0, t1));
			fmt::print("Setting 4 items to profile : {}\n", GetDuration(t1, t2));
			fmt::print("Save profile : {}\n", GetDuration(t2, t3));
		}

		{
			TLazyProfile<char> profile;
			auto r = std::filesystem::current_path();
			std::istringstream stream(strLazyProfile);
			auto t0 = std::chrono::steady_clock::now();
			profile.Load(stream);
			auto t1 = std::chrono::steady_clock::now();
			profile["Test"]("path") = 2;
			profile["Test"]("LogTranslatorEvents") = false;
			//profile[u8"Test"].SetItemValue(u8"String", u8"\" 바바바바바");	// error.
			profile["Test"].SetItemValue("newValue", 3.1415, "comment ___ new");

			profile["key having punctuation()- and spaces"]("path") = (char const*)u8"C:\\ABC\\DEF\\가나다라.txt";

			auto t2 = std::chrono::steady_clock::now();

			profile.Save("ini/lazy_profile.ini");

			auto t3 = std::chrono::steady_clock::now();

			auto GetDuration = [](auto t0, auto t1) {
				return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
			};
			fmt::print("Load profile : {}\n", GetDuration(t0, t1));
			fmt::print("Setting 4 items to profile : {}\n", GetDuration(t1, t2));
			fmt::print("Save profile : {}\n", GetDuration(t2, t3));
		}

		{
			TLazyProfile<wchar_t> profile;
			auto r = std::filesystem::current_path();
			std::istringstream stream(strLazyProfile);
			profile.Load(stream);

			profile.Save("ini/lazy_profileW.ini");
		}
	}

	TEST(misc, TContainerMap) {
		TContainerMap<std::vector, std::string, int> map;
		map.insert("z", 3);
		map.insert("o", 2);
		map.insert("o", 1);
		map.insert("b", 100);
		map.insert("a", 101);
		map["o"] = 0;

		EXPECT_EQ(map["z"], 3);
		EXPECT_EQ(map["o"], 0);
		EXPECT_EQ(map["b"], 100);
		EXPECT_EQ(map["a"], 101);

	}

	TEST(misc, IsChildPath) {
		EXPECT_TRUE(IsChildPath(L"C:/A/B/C/a.txt", L"C:/"));
		EXPECT_TRUE(IsChildPath(L"C:/A/B/C/a.txt", L"C:/A/B"));
		EXPECT_TRUE(IsChildPath(L"C:/A/B/C/a.txt", L"C:\\A\\"));
		EXPECT_TRUE(IsChildPath(L"C:/A/B/C/a.txt", L"C:/A/B/C"));
		EXPECT_TRUE(IsChildPath(L"C:/A/B/C/a.txt", L"C:/A/B/C/"));
		EXPECT_TRUE(IsChildPath(L"C:/A/B/C/a.txt", L"C:\\A\\B\\C\\"));
		EXPECT_TRUE(IsChildPath(L"C:/A/B/C/a.txt", L"C:\\A\\B\\C\\a.txt"));

		EXPECT_TRUE(IsChildPath(L"C:/A/B/C", L"C:/"));
		EXPECT_TRUE(IsChildPath(L"C:/A/B/C", L"C:/A/B"));
		EXPECT_TRUE(IsChildPath(L"C:/A/B/C", L"C:\\A\\"));
		EXPECT_TRUE(IsChildPath(L"C:/A/B/C", L"C:/A/B/C"));
		EXPECT_TRUE(IsChildPath(L"C:/A/B/C", L"C:/A/B/C/"));

		EXPECT_FALSE(IsChildPath(L"C:/A/B/C/a.txt", L"C:/A/B/C/a"));
		EXPECT_FALSE(IsChildPath(L"C:/A/B/C/a.txt", L"C:/A/B/C/b"));
		EXPECT_FALSE(IsChildPath(L"C:/A/B/C/a.txt", L"C:/A/B/C/d/"));
		EXPECT_FALSE(IsChildPath(L"C:/A/B/C/a.txt", L"C:/A/D/C"));
		EXPECT_FALSE(IsChildPath(L"D:/A/B/C/a.txt", L"C:/"));
		EXPECT_FALSE(IsChildPath(L"D:/A/B/C/a.txt", L"C:/A/B"));
		EXPECT_FALSE(IsChildPath(L"D:/A/B/C/a.txt", L"C:\\A\\B\\"));

		EXPECT_FALSE(IsChildPath(L"C:/A/B/C", L"C:/A/B/C/a"));
		EXPECT_FALSE(IsChildPath(L"C:/A/B/C", L"C:/A/B/C/b"));
		EXPECT_FALSE(IsChildPath(L"C:/A/B/C", L"C:/A/B/C/d/"));
		EXPECT_FALSE(IsChildPath(L"C:/A/B/C", L"C:/A/D/C"));
		EXPECT_FALSE(IsChildPath(L"D:/A/B/C", L"C:/"));
		EXPECT_FALSE(IsChildPath(L"D:/A/B/C", L"C:/A/B"));
		EXPECT_FALSE(IsChildPath(L"D:/A/B/C", L"C:\\A\\B\\"));
	}

	TEST(default_, enumerate_) {
		std::vector<int> lst{1,3,5,7,9};
		{
			size_t sum{};
			for (auto const [index, value] : std::views::enumerate(lst)) {
				sum += index;
			}
			EXPECT_EQ(sum, 0+1+2+3+4);
		}
		{
			int sum{};
			for (auto const [index, value] : enumerate_as<decltype(sum)>(lst)) {
				sum += index;
			}
			EXPECT_EQ(sum, 0+1+2+3+4);
		}
	}

	class xTestVirtualBase {
	public:
		std::vector<int> data;

		//xTestVirtualBase() = default;
		//xTestVirtualBase(xTestVirtualBase const&) = default;
		//xTestVirtualBase(xTestVirtualBase &&) = default;

		//virtual ~xTestVirtualBase() = default;
	};

	class xTestVirtualDerived : public xTestVirtualBase {
	public:
		std::vector<int> data2;

		virtual ~xTestVirtualDerived() = default;
	};

	void Func1(int, int, double) {}
	void Func1(int, double, int) {}


	TEST(misc, xFinalAction) {

		auto* p = std::bit_cast<int64_t*>(nullptr);

		int i{};
		{
			xFinalAction fa{[&]{ i = 3;} };

		}
		EXPECT_EQ(i, 3);


		//Func1(4, 5, 6);

		xTestVirtualBase ob1;
		ob1.data.push_back(1);
		ob1.data.push_back(2);
		ob1.data.push_back(3);
		auto ob2 = std::move(ob1);
		auto ob3 = ob1;

		EXPECT_EQ(ob1.data.size(), 0);
		EXPECT_EQ(ob2.data.size(), 3);

	}

	TEST(misc, matrix) {
		TMatrix<double, 3, 3> m1 { 1., 0., 0., 0., 2., 0., 0., 0., 3.}, m2{ 3., 0., 0., 0., 3., 0., 0., 0., 3.}, m4 { 3., 0., 0., 0., 6., 0., 0., 0., 9.};
		auto m3 = m1 * m2;
		EXPECT_EQ(m3, m4);
	}

	TEST(misc, ptr_container) {

		gtl::TConcurrentUPtrDeque<int> lst;

		lst.push_back(std::make_unique<int>(3));
		auto b = lst.begin();
		auto e = lst.end();
		auto rb = lst.rbegin();
		auto re = lst.rend();
		for (auto const& i : lst) {
			EXPECT_EQ(i, 3);
		}
		auto const& const_lst = lst;
		for (auto const& i : const_lst) {
			EXPECT_EQ(i, 3);
		}

		lst.clear();
		lst.push_back(std::make_unique<int>(10));
		lst.push_back(std::make_unique<int>(9));
		lst.push_back(std::make_unique<int>(8));
		lst.push_back(std::make_unique<int>(7));
		lst.push_back(std::make_unique<int>(6));
		lst.push_back(std::make_unique<int>(5));
		lst.push_back(std::make_unique<int>(4));
		lst.push_back(std::make_unique<int>(3));
		lst.push_back(std::make_unique<int>(2));
		lst.push_back(std::make_unique<int>(1));
		lst.push_back(std::make_unique<int>(0));

		//lst.push_front(std::make_unique<int>(0));

		std::sort(lst.base_t::begin(), lst.base_t::end(), [](auto const& a, auto const& b) { return *a<*b;});
		EXPECT_EQ(lst[0], 0);
		EXPECT_EQ(lst[10], 10);
		lst.clear();

		// test thread
		{
			constexpr size_t N = 100;
			gtl::TConcurrentUPtrDeque<int> lst;
			std::latch latchQueue(N);
			auto Queue = [&](size_t index) {
				latchQueue.count_down();
				latchQueue.wait();
				std::unique_lock lock(lst);
				for (int i = 0; i < N; i++) {
					lst.push_back(std::make_unique<int>((int)(index*N + i)));
				}
			};
			std::vector<std::thread> threadsQueue;
			threadsQueue.reserve(N);
			for (size_t i{}; i < N; i++) {
				threadsQueue.emplace_back(Queue, i);
			}
			for (auto& t : threadsQueue)
				t.join();

			std::latch latch(N);
			std::vector<std::thread> threads;
			threads.reserve(N);
			size_t sum{};
			for (size_t i{}; i < N; i++) {
				threads.emplace_back([&]{
					latch.count_down();
					latch.wait();
					std::unique_lock lock(lst);
					for (auto i : lst) {
						sum += i;
					}
				});
			}

			for (auto& t : threads)
				t.join();

			auto n = N*N-1;
			EXPECT_EQ(sum, ((1+n)*n/2)*N);
		}

		{
			constexpr size_t N = 100;
			gtl::TConcurrentUPtrDeque<int> lst;
			std::latch latchQueue(N);
			auto Queue = [&](size_t index) {
				latchQueue.count_down();
				latchQueue.wait();
				//std::unique_lock lock(lst);
				for (int i = 0; i < N; i++) {
					lst.push_back(std::make_unique<int>((int)(index*N + i)));
				}
			};

			std::vector<std::thread> threadsQueue;
			threadsQueue.reserve(N);
			for (size_t i{}; i < N; i++) {
				threadsQueue.emplace_back(Queue, i);
			}
			for (auto& t : threadsQueue)
				t.join();
			EXPECT_EQ(lst.size(), N*N);

			std::latch latch(N);
			std::vector<std::thread> threads;
			threads.reserve(N);
			size_t sum{};
			for (size_t i{}; i < N; i++) {
				threads.emplace_back([&]{
					latch.count_down();
					latch.wait();
					std::unique_lock lock(lst);
					for (auto i : lst) {
						sum += i;
					}
					});
			}

			for (auto& t : threads)
				t.join();

			auto n = N*N-1;
			EXPECT_EQ(sum, ((1+n)*n/2)*N);
		}
	}

	TEST(misc, StopWatch) {
		{
			gtl::TStopWatch<wchar_t> sw(std::wcout);
			sw.Lap(L"asdfasdf{}, {}", 1, L"asdf");
			sw.Lap(L"asdfasdf{}, {}", 2, L"asdf");
			sw.Lap(L"asdfasdf{}, {}", 3, L"가나다");
		}

		{
			gtl::TStopWatch sw(std::cout);
			sw.Lap("asdfasdf{}, {}", 1, "asdf");
			sw.Lap("asdfasdf{}, {}", 2, "asdf");
			sw.Lap("asdfasdf{}, {}", 3, "가나다");
		}
	}
}


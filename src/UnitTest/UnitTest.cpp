#include "pch.h"
#include "CppUnitTest.h"

//#include "gtl/string.h"
#include "gtl/gtl.h"
import gtl;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//template fmt::v7::detail::buffer_appender<wchar_t>;
//template fmt::v7::detail::dragonbox::decimal_fp<float>;
//template fmt::v7::detail::dragonbox::decimal_fp<double>;
//template fmt::v7::detail::dragonbox::decimal_fp<float> fmt::v7::detail::dragonbox::to_decimal<float>(float);
//template fmt::v7::detail::dragonbox::decimal_fp<double> fmt::v7::detail::dragonbox::to_decimal<double>(double);
//fmt::v7::detail::buffer_appender<char8_t>;

namespace Microsoft::VisualStudio::CppUnitTestFramework {
	template<> inline std::wstring ToString<char8_t>               (char8_t const& t)               { return fmt::format(L"{}", (int)t); }
	//template<> inline std::wstring ToString<char8_t>               (char8_t const* t)               { return fmt::format(L"{}", t); }
	//template<> inline std::wstring ToString<char8_t>               (char8_t* t)                     { return fmt::format(L"{}", t); }
	template<> inline std::wstring ToString<char16_t>              (char16_t const& t)              { return fmt::format(L"{}", (int)t); }
	//template<> inline std::wstring ToString<char16_t>              (char16_t const* t)              { return fmt::format(L"{}", t); }
	//template<> inline std::wstring ToString<char16_t>              (char16_t* t)                    { return fmt::format(L"{}", t); }
	template<> inline std::wstring ToString<char32_t>              (char32_t const& t)              { return fmt::format(L"{}", (int)t); }
	//template<> inline std::wstring ToString<char32_t>              (char32_t const* t)              { return fmt::format(L"{}", t); }
	//template<> inline std::wstring ToString<char32_t>              (char32_t* t)                    { return fmt::format(L"{}", t); }
}

namespace UnitTest {


	//BEGIN_TEST_MODULE_ATTRIBUTE()
	//	TEST_MODULE_ATTRIBUTE(L"Date", L"2020/11/03")
	//END_TEST_MODULE_ATTRIBUTE()


	//TEST_MODULE_INITIALIZE(ModuleInitialize) {
	//	Logger::WriteMessage("In Module Initialize");
	//}

	//TEST_MODULE_CLEANUP(ModuleCleanup) {
	//	Logger::WriteMessage("In Module Cleanup");
	//}



	//TEST_CLASS(Class1) {
	//public:
	//	Class1() {
	//		Logger::WriteMessage("In Class1");
	//	}
	//	~Class1() {
	//		Logger::WriteMessage("In ~Class1");
	//	}



	//	TEST_CLASS_INITIALIZE(ClassInitialize) {
	//		Logger::WriteMessage("In Class Initialize");
	//	}

	//	TEST_CLASS_CLEANUP(ClassCleanup) {
	//		Logger::WriteMessage("In Class Cleanup");
	//	}


	//	BEGIN_TEST_METHOD_ATTRIBUTE(Method1)
	//		TEST_OWNER(L"OwnerName")
	//		TEST_PRIORITY(1)
	//	END_TEST_METHOD_ATTRIBUTE()

	//	TEST_METHOD(Method1) {
	//		Logger::WriteMessage("In Method1");
	//		Assert::AreEqual(1, 1);
	//	}

	//	TEST_METHOD(Method2) {
	//		Assert::Fail(L"Fail");
	//	}
	//};

	TEST_CLASS(UnitTest) {
	public:

		TEST_METHOD(TestMethod1) {
			//std::pmr::memory_resource;
			//std::pmr::string str;

			using namespace std::literals;
			using namespace gtl::literals;

			//std::filesystem::path path;
			//path.wstring();
			//path.string();
			//path.u8string();
			//path.u16string();
			//path.u32string();

			std::string str1{ "message1" };
			gtl::xStringA str2 = gtl::ToString<char, char>(str1);
			gtl::xStringA str3 = "asdfasdjklfd";
			gtl::xString str4{ gtl::xStringA(str1) };

			std::string_view sv{ str1 };
			std::string_view sv2{ str2 };
			std::wstring_view sv3{ str4 };

			auto stru = fmt::format(u8"{}", 1);
			auto strU16 = fmt::format(u"{}", 2);
			auto strU32 = fmt::format(U"{}", 3);


			gtl::xStringA str5{ sv };
			gtl::xString str6(sv);
			str6 = sv;
			str6 = str5;
			str6 = str1;
			str2 = str6;
			str2 = str3;

			str2 = "abcde";
			Assert::AreEqual('b', str2[1]);
			str2 = L"abcde";
		}
		TEST_METHOD(tsztoi) {
			using namespace std::literals;
			using namespace gtl::literals;

			try {
				Assert::AreEqual(123456, gtl::tsztoi(std::basic_string("123456")));
				Assert::AreEqual(123456, gtl::tsztoi("123456"s));
				Assert::AreEqual(123456, gtl::tsztoi("123456"sv));
				Assert::AreEqual(123456, gtl::tsztoi("123456"sv));
				auto i = gtl::tsztoi<int64_t>("0xABCD'EFab"sv);
				Assert::AreEqual(0xABCD'EFabI64, gtl::tsztoi<int64_t>("0xABCD'EFab"sv));

				double v = gtl::tsztod("1.1"sv);
				Assert::AreEqual(1.1, gtl::tsztod("1.1"sv));

			}
			catch (...) {

			}
		}

		TEST_METHOD(Test2) {

			constexpr char16_t cLeading{ 0xd801 };
			constexpr char16_t cTrailing{ 0xdc03 };
			constexpr char32_t c32{ (1 << 10 | 03) + 0x1'0000 };

			constexpr auto c = ((cLeading - 0xd800) << 10) + (cTrailing - 0xdc00) + 0x1'0000;
			constexpr char32_t adder = 0x1'0000 -((0xd800 << 10) + 0xdc00);
			constexpr auto c_ = adder + (cLeading << 10) + cTrailing;
			constexpr auto c2 = 0xfca0'2400u + (cLeading << 10) + cTrailing;

			Assert::AreEqual(c32, (char32_t)c);
			Assert::AreEqual((uint32_t)c32, (uint32_t)c2);
		}
	};
}

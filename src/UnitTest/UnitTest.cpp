#include "pch.h"
#include "CppUnitTest.h"

#include "gtl/string.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

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

			//std::filesystem::path path;
			//path.wstring();
			//path.string();
			//path.u8string();
			//path.u16string();
			//path.u32string();

			std::string str1 { "message1" };
			gtl::CStringA str2 = gtl::ToString<char, char>(str1);
			gtl::CStringA str3 = "asdfasdjklfd";
			gtl::CString str4{gtl::CStringA(str1)};

			std::string_view sv {str1};
			std::string_view sv2 {str2};
			std::u16string_view sv3 {str4};

			gtl::CStringA str5 {sv};
			gtl::CString str6(sv);
			str6 = sv;
			str6 = str5;
			str6 = str1;
			str2 = str6;
			str2 = str3;

			str2 = "abcde";
			Assert::AreEqual('b', str2[1]);
			str2 = L"abcde";
		}
	};
}

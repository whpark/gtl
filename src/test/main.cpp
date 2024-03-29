﻿#include "pch.h"
#include "gtl/misc.h"

#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include "fmt/xchar.h"

// vcpkg does this.
//#ifdef _DEBUG
//#pragma comment(lib, "gtest.lib")
//#pragma comment(lib, "gmock.lib")
//#else
//#pragma comment(lib, "gtest.lib")
//#pragma comment(lib, "gmock.lib")
//#endif
#pragma comment(lib, "Shlwapi.lib")


namespace testing::internal {

	//void PrintU8StringTo(const ::std::u8string& s, ::std::ostream* os) {
	//	for (auto c : s) {
	//		*os << std::hex << (int) c << ", ";
	//	}
	//	*os << std::dec;
	//}

}


int main(int argc, wchar_t** argv) {
	testing::InitGoogleTest(&argc, argv);
	EXPECT_TRUE(std::setlocale(LC_ALL, "Korean.949"));

	gtl::SetCurrentPath_ProjectFolder();
	fmt::print("Current Folder : {}\n\n", std::filesystem::current_path().string().c_str());
	auto r = RUN_ALL_TESTS();
	return r;
}

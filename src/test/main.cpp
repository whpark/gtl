﻿#include "pch.h"


#ifdef _DEBUG
#pragma comment(lib, "gtestd.lib")
#pragma comment(lib, "gmockd.lib")
#else
#pragma comment(lib, "gtest.lib")
#pragma comment(lib, "gmock.lib")
#endif
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
	RUN_ALL_TESTS();
}

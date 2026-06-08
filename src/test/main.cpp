#include "pch.h"
#include "gtl/gtl.h"

//#pragma comment(lib, "Shlwapi.lib")

namespace testing::internal {

	//void PrintU8StringTo(const ::std::u8string& s, ::std::ostream* os) {
	//	for (auto c : s) {
	//		*os << std::hex << (int) c << ", ";
	//	}
	//	*os << std::dec;
	//}

}


int main(int argc, wchar_t* argv[]) {
	testing::InitGoogleTest(&argc, argv);
	EXPECT_TRUE(std::setlocale(LC_ALL, "Korean.949"));

	gtl::SetCurrentPath_ProjectFolder();
	fmt::println("Current Folder : {}\n", std::filesystem::current_path());
	auto r = RUN_ALL_TESTS();
	return r;
}


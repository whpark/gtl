#include "pch.h"
#include "gtl/misc.h"

#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include "fmt/xchar.h"

int main(int argc, char** argv) {
	gtl::SetCurrentPath_ProjectFolder();
	fmt::print("Current Folder : {}\n\n", std::filesystem::current_path().string().c_str());
	return Catch::Session().run(argc, argv);
}

#include "pch.h"

#include <clocale>

#include <catch2/catch_session.hpp>

#include "fmt/format.h"
#include "fmt/std.h"		// std::filesystem::path formatter

#include "gtl/misc.h"

int main(int argc, char* argv[]) {
	if (!std::setlocale(LC_ALL, "Korean.949")) {
		fmt::println(stderr, "Failed to set locale: Korean.949");
		return 1;
	}

	// -> <gtl root>/src/test.dxf
	gtl::SetCurrentPath_ProjectFolder();
	fmt::println("Current Folder : {}\n", std::filesystem::current_path());

	Catch::Session session;
	session.configData().skipBenchmarks = true;
	return session.run(argc, argv);
}


#include "pch.h"
#include "gtl/gtl.h"

#include <catch2/catch_session.hpp>

//#pragma comment(lib, "Shlwapi.lib")

int main(int argc, char* argv[]) {
	if (!std::setlocale(LC_ALL, "Korean.949")) {
		fmt::println(stderr, "Failed to set locale: Korean.949");
		return 1;
	}

	gtl::SetCurrentPath_ProjectFolder();
	fmt::println("Current Folder : {}\n", std::filesystem::current_path());

	fmt::print(fmt::emphasis::bold|fmt::fg(fmt::rgb{0,255,0}), "Hello world {{0x{:06x}}}\n", 0x00'ff'00);
	fmt::print(fmt::emphasis::bold|fmt::fg(fmt::terminal_color::green), "Hello world {{{}}}\n", "TerminalGreen");
	fmt::print(fmt::emphasis::bold|fmt::fg(fmt::color::green), "Hello world {{{}}}\n", "Color::Green");

	return Catch::Session().run(argc, argv);
}


// bench.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <iostream>

#include <catch2/catch_session.hpp>
#include "gtl/gtl.h"
#pragma comment(lib, "shlwapi.lib")

int main(int argc, char** argv) {

	gtl::SetCurrentPath_ProjectFolder();
	return Catch::Session().run(argc, argv);
}

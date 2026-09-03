#include "gtl/gtl.h"

#include <catch2/catch_session.hpp>

int main(int argc, char* argv[]) {
	gtl::SetCurrentPath_ProjectFolder();
	return Catch::Session().run(argc, argv);
}


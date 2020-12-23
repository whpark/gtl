// bench.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <iostream>

//#define FMT_HEADER_ONLY
//#include "fmt/color.h"
//#include "fmt/ostream.h"

#include "benchmark/benchmark.h"
#pragma comment(lib, "benchmark.lib")
#pragma comment(lib, "shlwapi.lib")

int main(int argc, char** argv) {
    //std::cout << "Hello World! {cout}\n";
    //fmt::print(fmt::emphasis::bold|fmt::fg(fmt::v7::rgb{0,255,0}), "Hello world {{0x{:06x}}}\n", 0x00'ff'00);
    //fmt::print(fmt::emphasis::bold|fmt::fg(fmt::terminal_color::green), "Hello world {{{}}}\n", "TerminalGreen");
    //fmt::print(fmt::emphasis::bold|fmt::fg(fmt::color::green), "Hello world {{{}}}\n", "Color::Green");

	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();

	return 0;
}


//// Define a function that executes the code to be measured a
//// specified number of times:
//static void BM_StringCreation(benchmark::State& state) {
//  for (auto _ : state)
//    std::string empty_string;
//}
//
//// Register the function as a benchmark
//BENCHMARK(BM_StringCreation);
//

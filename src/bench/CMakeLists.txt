# bench executable (was: bench.vcxproj) -- no PCH in the original project.

find_package(Catch2 CONFIG REQUIRED)

add_executable(bench
	bench.cpp
	bench_string_codepage_conv.cpp
	$<$<BOOL:${WIN32}>:bench.rc>
)

target_include_directories(bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(bench PRIVATE _CONSOLE)

target_link_libraries(bench PRIVATE gtl Catch2::Catch2)

gtl_apply_common_options(bench)
gtl_set_exe_output_dir(bench)

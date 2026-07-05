# test_sequence executable (was: test_sequence.vcxproj)

find_package(Catch2 CONFIG REQUIRED)

add_executable(test_sequence
	main.cpp
	pch.cpp
	test_sequence.cpp
	test_sequence_map.cpp
	test_sequence_tReturn.cpp
	$<$<BOOL:${WIN32}>:test_sequence.rc>
)

target_include_directories(test_sequence PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(test_sequence PRIVATE _CONSOLE X64)

target_precompile_headers(test_sequence PRIVATE pch.h)

target_link_libraries(test_sequence PRIVATE gtl Catch2::Catch2)

gtl_apply_common_options(test_sequence)
gtl_set_exe_output_dir(test_sequence)

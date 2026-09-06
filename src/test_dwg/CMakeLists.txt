find_package(Catch2 CONFIG REQUIRED)

add_executable(test.dwg main.cpp test_dwg.cpp pch.cpp ../dwg/detail/container.cpp ../dwg/detail/reader.cpp)
set_source_files_properties(../dwg/detail/container.cpp ../dwg/detail/reader.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
target_include_directories(test.dwg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_precompile_headers(test.dwg PRIVATE pch.h)
target_compile_definitions(test.dwg PRIVATE _CRT_SECURE_NO_WARNINGS GTL_DWG_SAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/DWG")
target_link_libraries(test.dwg PRIVATE gtl gtl.shape gtl.dwg Catch2::Catch2)

gtl_apply_common_options(test.dwg)
gtl_set_exe_output_dir(test.dwg)
# Keep both configurations identifiable in GTL's shared bin directory. Otherwise
# an up-to-date Debug build can accidentally run the last Release executable.
set_target_properties(test.dwg PROPERTIES
	OUTPUT_NAME_DEBUG "test.dwg.x64D"
	OUTPUT_NAME_RELEASE "test.dwg.x64R"
	OUTPUT_NAME_RELWITHDEBINFO "test.dwg.x64R"
	OUTPUT_NAME_MINSIZEREL "test.dwg.x64R"
)
add_test(NAME gtl.dwg COMMAND test.dwg)

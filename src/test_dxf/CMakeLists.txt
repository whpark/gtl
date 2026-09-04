# test.dxf executable (was: biscuit/src/test.dxf)

find_package(Catch2 CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

add_executable(test.dxf
	main.cpp
	pch.cpp
	test.dxf.cpp
)

target_include_directories(test.dxf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(test.dxf PRIVATE _CONSOLE X64 _CRT_SECURE_NO_WARNINGS)

target_precompile_headers(test.dxf PRIVATE pch.h)

target_link_libraries(test.dxf PRIVATE gtl gtl.shape gtl.dxf Catch2::Catch2 magic_enum::magic_enum)

gtl_apply_common_options(test.dxf)
gtl_set_exe_output_dir(test.dxf)

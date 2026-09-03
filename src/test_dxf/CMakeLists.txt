# test.dxf - tests and sample reader for gtl.dxf.

find_package(Catch2 CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

add_executable(test.dxf
	main.cpp
	test_dxf.cpp
)

target_compile_definitions(test.dxf PRIVATE _CONSOLE X64)

target_link_libraries(test.dxf PRIVATE
	gtl.dxf
	Catch2::Catch2
	magic_enum::magic_enum
)

gtl_apply_common_options(test.dxf)
gtl_set_exe_output_dir(test.dxf)


# test executable (was: test.vcxproj)

find_package(Catch2 CONFIG REQUIRED)

add_executable(test
	main.cpp
	pch.cpp
	test_archive.cpp
	test_boost_ptr_container.cpp
	test_coord.cpp
	test_coord_trans.cpp
	test_dynamic.cpp
	test_json_proxy.cpp
	test_lock.cpp
	test_misc.cpp
	test_mutex.cpp
	test_reflection.cpp
	test_reflection_glaze.cpp
	test_shape.cpp
	test_string.cpp
	test_string_codepage.cpp
	test_string_primitives.cpp
	test_task.cpp
	$<$<BOOL:${WIN32}>:test.rc>
)

target_include_directories(test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(test PRIVATE _CONSOLE X64)

target_precompile_headers(test PRIVATE pch.h)

target_link_libraries(test PRIVATE gtl gtl.shape Catch2::Catch2)

gtl_apply_common_options(test)
gtl_set_exe_output_dir(test)

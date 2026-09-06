add_library(gtl.dwg SHARED dwg.cpp detail/reader.cpp detail/container.cpp pch.cpp
	detail/curves.h detail/hatch.h detail/header_fields.inc)

target_include_directories(gtl.dwg
	PUBLIC ${CMAKE_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(gtl.dwg PRIVATE GTL__DWG_EXPORTS)
target_precompile_headers(gtl.dwg PRIVATE pch.h)
set_source_files_properties(detail/reader.cpp detail/container.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
target_link_libraries(gtl.dwg PUBLIC gtl gtl.shape)

gtl_apply_common_options(gtl.dwg)
gtl_set_output_name(gtl.dwg "gtl.dwg")

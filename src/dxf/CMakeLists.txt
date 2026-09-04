# gtl.dxf (was: gtl.dxf.vcxproj -> gtl.dxf.x64R.dll / gtl.dxf.x64D.dll)
#
# gtl/dxf/*.h is a header-only implementation; this library only builds the
# light-weight entry point declared in gtl/dxf/dxf_reader.h.

add_library(gtl.dxf SHARED
	dllmain.cpp
	dxf.cpp
	pch.cpp
)

target_include_directories(gtl.dxf
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(gtl.dxf PRIVATE GTL__DXF_EXPORTS _USRDLL _WINDOWS)

target_precompile_headers(gtl.dxf PRIVATE pch.h)

target_link_libraries(gtl.dxf
	PUBLIC gtl gtl.shape
)

gtl_apply_common_options(gtl.dxf)
gtl_set_output_name(gtl.dxf "gtl.dxf")

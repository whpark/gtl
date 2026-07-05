# gtl.shape (was: shape.vcxproj -> gtl.shape.x64R.dll / gtl.shape.x64D.dll)

# The vcpkg tinyspline port ships tinysplinecxx (the C++ wrapper canvas.cpp actually
# includes) as a separate DLL/import lib with no CMake config of its own.
find_package(tinyspline CONFIG REQUIRED)
find_library(TINYSPLINECXX_LIBRARY NAMES tinysplinecxx REQUIRED)

add_library(gtl.shape SHARED
	canvas.cpp
	dllmain.cpp
	pch.cpp
	shape.cpp
	$<$<BOOL:${WIN32}>:gtl.shape.rc>
)

target_include_directories(gtl.shape
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(gtl.shape PRIVATE GTL__SHAPE_EXPORTS _USRDLL _WINDOWS)

target_precompile_headers(gtl.shape PRIVATE pch.h)

target_link_libraries(gtl.shape
	PUBLIC gtl
	PRIVATE tinyspline::tinyspline ${TINYSPLINECXX_LIBRARY}
)

gtl_apply_common_options(gtl.shape)
gtl_set_output_name(gtl.shape "gtl.shape")

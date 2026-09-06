# gtl.shape (was: shape.vcxproj -> gtl.shape.x64R.dll / gtl.shape.x64D.dll)

# The vcpkg tinyspline port ships tinysplinecxx (the C++ wrapper canvas.cpp actually
# includes) as a separate DLL/import lib with no CMake config of its own.
find_package(tinyspline CONFIG REQUIRED)
foreach(config RELEASE DEBUG)
	get_target_property(tinyspline_library tinyspline::tinyspline IMPORTED_IMPLIB_${config})
	if(NOT tinyspline_library)
		get_target_property(tinyspline_library tinyspline::tinyspline IMPORTED_LOCATION_${config})
	endif()
	get_filename_component(tinyspline_library_dir "${tinyspline_library}" DIRECTORY)
	find_library(TINYSPLINECXX_LIBRARY_${config} NAMES tinysplinecxx
		HINTS "${tinyspline_library_dir}" NO_DEFAULT_PATH REQUIRED)
endforeach()
include(SelectLibraryConfigurations)
select_library_configurations(TINYSPLINECXX)

add_library(gtl.shape SHARED
	canvas.cpp
	dllmain.cpp
	pch.cpp
	shape.cpp
	$<$<BOOL:${WIN32}>:gtl.shape.rc>
)

# Both vcpkg configurations use the same runtime DLL name. The shared GTL bin
# directory must receive the matching C++ ABI even when switching configurations
# without relinking this target.
if(WIN32)
	get_filename_component(tinysplinecxx_release_dir "${TINYSPLINECXX_LIBRARY_RELEASE}" DIRECTORY)
	get_filename_component(tinysplinecxx_debug_dir "${TINYSPLINECXX_LIBRARY_DEBUG}" DIRECTORY)
	add_custom_target(gtl.shape.runtime
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:gtl.shape>"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"$<IF:$<CONFIG:Debug>,${tinysplinecxx_debug_dir},${tinysplinecxx_release_dir}>/../bin/tinysplinecxx.dll"
			"$<TARGET_FILE_DIR:gtl.shape>/tinysplinecxx.dll"
		VERBATIM)
	add_dependencies(gtl.shape gtl.shape.runtime)
endif()

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

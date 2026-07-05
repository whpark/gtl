# gtl core library (was: gtl.vcxproj -> gtl.x64R.dll / gtl.x64D.dll)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(ctre CONFIG REQUIRED)
find_package(boost_json CONFIG REQUIRED)
find_package(boost_serialization CONFIG REQUIRED)
find_package(boost_ptr_container CONFIG REQUIRED)
find_package(boost_variant CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(LibArchive REQUIRED)
find_package(Iconv REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(freeimage CONFIG REQUIRED)

add_library(gtl SHARED
	2dMatArray.cpp
	archive.cpp
	cv_floodfill.cpp
	dllmain.cpp
	HangeulCodeMapKSSM_UTF16.cpp
	HangeulCodeMapUTF16_KSSM.cpp
	json_proxy.cpp
	log.cpp
	mat_gl.cpp
	mat_helper.cpp
	pch.cpp
	string.cpp
	$<$<BOOL:${WIN32}>:gtl.rc>
)

target_include_directories(gtl
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(gtl PRIVATE GTL__EXPORTS _USRDLL _WINDOWS)

target_precompile_headers(gtl PRIVATE pch.h)

# Public: types from these headers appear in gtl's public headers (gtl.h -> mat_helper.h,
# json_proxy.h, archive.h, ...), so consumers need them on their include/link path too.
target_link_libraries(gtl PUBLIC
	fmt::fmt
	spdlog::spdlog
	nlohmann_json::nlohmann_json
	glaze::glaze
	ctre::ctre
	Boost::json
	Boost::serialization
	Boost::ptr_container
	Boost::variant
	${OpenCV_LIBS}
	OpenGL::GL
	Iconv::Iconv
)
target_include_directories(gtl PUBLIC ${OpenCV_INCLUDE_DIRS})

# Private: implementation details only (FIBITMAP is forward-declared in the public header).
target_link_libraries(gtl PRIVATE
	LibArchive::LibArchive
	GLEW::glew
	freeimage::FreeImage
)

gtl_apply_common_options(gtl)
gtl_set_output_name(gtl "gtl")

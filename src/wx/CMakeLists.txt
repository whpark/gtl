# gtl.wx (was: wx.vcxproj -> gtl.wx.x64R.dll / gtl.wx.x64D.dll)
#
# MatView/MatView.cpp, MatView/wxMatView.cpp, MatView/DlgMatViewOption.cpp and the public
# header include/gtl/wx/MatView.h hard-depend on Skia (SkCanvas/SkSurface/sk_sp<SkTypeface>
# member variables). Skia isn't in this repo's vcpkg dependency list and has no vcpkg
# CMake config here -- it's built out-of-tree (see D:/Project3rd/skia on this machine).
# Keep it opt-in via GTL_WX_ENABLE_SKIA until a Skia build/toolchain is wired up.

find_package(wxWidgets CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

set(gtl_wx_sources
	dllmain.cpp
	pch.cpp
	util.cpp
	wxRoundButton.cpp
)

if(GTL_WX_ENABLE_SKIA)
	set(GTL_SKIA_ROOT "D:/Project3rd/skia" CACHE PATH "Skia checkout/build root (needs include/ and out/<config>/ with skia.lib)")
	list(APPEND gtl_wx_sources
		MatView/DlgMatViewOption.cpp
		MatView/MatView.cpp
		MatView/wxGLCanvasAdapter.cpp
		MatView/wxMatView.cpp
	)
endif()

add_library(gtl.wx SHARED
	${gtl_wx_sources}
	$<$<BOOL:${WIN32}>:gtl.wx.rc>
)

target_include_directories(gtl.wx
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

if(GTL_WX_ENABLE_SKIA)
	target_compile_definitions(gtl.wx PUBLIC GTL__WX_ENABLE_SKIA)
	target_include_directories(gtl.wx PUBLIC "${GTL_SKIA_ROOT}" "${GTL_SKIA_ROOT}/include")
endif()

target_compile_definitions(gtl.wx PRIVATE GTL__WX_EXPORTS _USRDLL _WINDOWS)

target_precompile_headers(gtl.wx PRIVATE pch.h)

target_link_libraries(gtl.wx
	PUBLIC
		gtl
		wx::wxbase
		wx::wxcore
		wx::wxadv
		wx::wxgl
		wx::wxwebview
		wx::wxxml
		OpenGL::GL
	PRIVATE
		GLEW::glew
)

gtl_apply_common_options(gtl.wx)
gtl_set_output_name(gtl.wx "gtl.wx")

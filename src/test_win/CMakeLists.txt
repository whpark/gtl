# test_win executable (was: test_win.vcxproj) -- full MFC SDI/MDI sample app.
# MSVC-only, see GTL_BUILD_MFC in the root CMakeLists.

set(CMAKE_MFC_FLAG 2)

find_package(scn CONFIG REQUIRED)

add_executable(test_win WIN32
	calendarbar.cpp
	ChildFrm.cpp
	ClassView.cpp
	FileView.cpp
	MainFrm.cpp
	OutputWnd.cpp
	pch.cpp
	PropertiesWnd.cpp
	test_win.cpp
	test_winDoc.cpp
	test_winView.cpp
	ViewTree.cpp
	testwin.rc
)

target_include_directories(test_win PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(test_win PRIVATE _WINDOWS)

target_precompile_headers(test_win PRIVATE pch.h)

target_link_libraries(test_win PRIVATE gtl gtl.mfc gtl.shape gtl.win scn::scn)

gtl_apply_common_options(test_win)
gtl_set_exe_output_dir(test_win)

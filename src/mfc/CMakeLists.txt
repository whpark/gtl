# gtl.mfc (was: mfc.vcxproj -> gtl.mfc.x64R.dll / gtl.mfc.x64D.dll)
# MFC Extension DLL. MSVC-only (MFC has no MinGW/g++ support), so this is never part of
# the eventual MSYS2 ucrt64 build -- see GTL_BUILD_MFC in the root CMakeLists.

# CMAKE_MFC_FLAG is a directory-scoped setting read when add_library()/add_executable()
# is called, not a target property, so it must be set before the target is declared.
# 2 == "Use MFC in a Shared DLL" (matches the vcxproj's <UseOfMfc>Dynamic</UseOfMfc>).
set(CMAKE_MFC_FLAG 2)

# OpenCV (opencv2/opencv.hpp is used directly in MatHelper.cpp/MatView.cpp) comes in
# transitively via the PUBLIC link to gtl below -- no separate find_package needed here.

add_library(gtl.mfc SHARED
	dllmain.cpp
	ListCtrlEd.cpp
	MatHelper.cpp
	MatView.cpp
	ProgressDlg.cpp
	TMDialog.cpp
	mfc.cpp
	pch.cpp
	mfc.rc
	mfc.def
)

target_include_directories(gtl.mfc
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(gtl.mfc PRIVATE GTL__MFC_EXPORTS _AFXEXT _WINDOWS)

target_precompile_headers(gtl.mfc PRIVATE pch.h)
# dllmain.cpp was NotUsingPrecompiledHeaders in the vcxproj (it still textually
# #include "pch.h", just not through the shared .pch binary).
set_source_files_properties(dllmain.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

target_link_libraries(gtl.mfc PUBLIC gtl gtl.win)

gtl_apply_common_options(gtl.mfc)
gtl_set_output_name(gtl.mfc "gtl.mfc")

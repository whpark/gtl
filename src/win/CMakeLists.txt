# gtl.win: Windows API helpers (was: win.vcxproj -> gtl.win.x64R.dll / gtl.win.x64D.dll)

find_path(WINREG_INCLUDE_DIR NAMES "winreg/WinReg.hpp")

add_library(gtl.win SHARED
	dllmain.cpp
	EnvironmentVariable.cpp
	pch.cpp
	serial_port.cpp
	win.cpp
	$<$<BOOL:${WIN32}>:gtl.win.rc>
)

target_include_directories(gtl.win
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
		${WINREG_INCLUDE_DIR}
)

target_compile_definitions(gtl.win PRIVATE GTL__WIN_EXPORTS _USRDLL _WINDOWS)

target_precompile_headers(gtl.win PRIVATE pch.h)

target_link_libraries(gtl.win PUBLIC gtl)

gtl_apply_common_options(gtl.win)
gtl_set_output_name(gtl.win "gtl.win")

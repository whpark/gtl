# test_qt executable (was: test_qt.vcxproj)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(test_qt WIN32
	main.cpp
	pch.cpp
	test_qt.cpp
	LayeredMatViewDlg.cpp
	SeqChildDlg.cpp
	SeqMainDlg.cpp
	TestSeqDlg.cpp

	test_qt.h
	LayeredMatViewDlg.h
	SeqChildDlg.h
	SeqMainDlg.h
	TestSeqDlg.h

	test_qt.ui
	LayeredMatViewDlg.ui
	SeqChildDlg.ui
	SeqMainDlg.ui
	TestSeqDlg.ui

	test_qt.qrc
	$<$<BOOL:${WIN32}>:test_qt.rc>
)

target_include_directories(test_qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(test_qt PRIVATE pch.h)

target_link_libraries(test_qt PRIVATE
	gtl
	gtl.shape
	gtl.win
	gtl.qt
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
	Qt6::OpenGL
	Qt6::OpenGLWidgets
)

gtl_apply_common_options(test_qt)
gtl_set_exe_output_dir(test_qt)

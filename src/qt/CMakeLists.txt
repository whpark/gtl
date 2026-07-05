# gtl.qt (was: qt.vcxproj -> gtl.qt.x64R.dll / gtl.qt.x64D.dll)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
find_package(glaze CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(gtl.qt SHARED
	MatBitmapArchive.cpp
	pch.cpp
	ProgressDlg.cpp
	QCollapsibleGroupBox.cpp
	QColorButton.cpp
	QFileFolderBrowser.cpp
	QGlazeModel.cpp
	QPathCompleter.cpp
	QRoundButton.cpp
	qt.cpp
	util.cpp
	MatView/LayeredMatView.cpp
	MatView/LayeredMatViewDlg.cpp
	MatView/MatView.cpp
	MatView/MatViewDlg.cpp
	MatView/MatViewGV.cpp
	MatView/MatViewGVCanvas.cpp
	MatView/MatViewGVDlg.cpp
	MatView/MatViewGVSettingsDlg.cpp
	MatView/MatViewGVSettingsDlg.h
	MatView/MatViewGVToolbar.cpp
	MatView/MatViewGVToolbar.h
	MatView/MatViewSettingsDlg.cpp
	MatView/MatViewSettingsDlg.h

	# Q_OBJECT headers that live under the public include/ tree (AUTOMOC only scans a
	# .cpp's own directory for a matching private header, so these must be listed here).
	${CMAKE_SOURCE_DIR}/include/gtl/qt/QFileFolderBrowser.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/QColorButton.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/QRoundButton.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/ProgressDlg.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/QPathCompleter.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/QGlazeModel.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/QCollapsibleGroupBox.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/MatView/LayeredMatView.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/MatView/LayeredMatViewDlg.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/MatView/MatViewGVCanvas.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/MatView/MatViewGVDlg.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/MatView/MatViewGV.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/MatView/MatView.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/MatView/MatViewDlg.h
	${CMAKE_SOURCE_DIR}/include/gtl/qt/MatView/MatViewCanvas.h

	ProgressDlg.ui
	QFileFolderBrowser.ui
	MatView/LayeredMatView.ui
	MatView/MatView.ui
	MatView/MatViewDlg.ui
	MatView/MatViewGV.ui
	MatView/MatViewGVDlg.ui
	MatView/MatViewGVSettingsDlg.ui
	MatView/MatViewGVToolbar.ui
	MatView/MatViewSettingsDlg.ui

	$<$<BOOL:${WIN32}>:gtl.qt.rc>
)

target_include_directories(gtl.qt
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/MatView
)

target_compile_definitions(gtl.qt PRIVATE GTL__QT_EXPORTS _USRDLL _WINDOWS)

target_precompile_headers(gtl.qt PRIVATE pch.h)

target_link_libraries(gtl.qt
	PUBLIC
		gtl
		gtl.shape
		Qt6::Core
		Qt6::Gui
		Qt6::Widgets
		Qt6::OpenGL
		Qt6::OpenGLWidgets
	PRIVATE
		glaze::glaze
		GLEW::glew
)

gtl_apply_common_options(gtl.qt)
gtl_set_output_name(gtl.qt "gtl.qt")

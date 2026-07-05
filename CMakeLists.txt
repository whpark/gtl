cmake_minimum_required(VERSION 3.24)

# vcpkg toolchain must be picked up before project(). If the caller already passed
# -DCMAKE_TOOLCHAIN_FILE=..., that takes precedence; otherwise fall back to $VCPKG_ROOT.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")
endif()

project(gtl LANGUAGES CXX)

if(WIN32)
	enable_language(RC)
endif()

# The original solution only ever had Debug|x64 and Release|x64 configurations.
if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
endif()
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Match the old post-build steps that copied outputs into <solution>/bin and <solution>/lib.
set(GTL_BIN_DIR "${CMAKE_SOURCE_DIR}/bin" CACHE PATH "Output directory for executables/DLLs")
set(GTL_LIB_DIR "${CMAKE_SOURCE_DIR}/lib" CACHE PATH "Output directory for import libraries")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GtlCommon)
include(CMakeDependentOption)

# --- module toggles -----------------------------------------------------------------
option(GTL_BUILD_WIN          "Build gtl.win (Windows API helpers)"        ON)
option(GTL_BUILD_SHAPE        "Build gtl.shape"                            ON)
option(GTL_BUILD_WX           "Build gtl.wx (wxWidgets UI module)"         ON)
option(GTL_BUILD_QT           "Build gtl.qt (Qt6 UI module)"               ON)
option(GTL_BUILD_TEST         "Build the catch2-based 'test' executable"   ON)
option(GTL_BUILD_BENCH        "Build the 'bench' executable"               ON)
option(GTL_BUILD_TEST_SEQUENCE "Build the 'test_sequence' executable"      ON)
option(GTL_BUILD_TEST_QT      "Build the 'test_qt' executable"             ON)
option(GTL_WX_ENABLE_SKIA     "Build wx/MatView/MatView.cpp (needs a Skia build not managed by vcpkg)" OFF)

# MFC has no MinGW/g++ support at all, and clang-cl doesn't officially support the MS
# extensions ATL/MFC's headers rely on, so these two only make sense with real cl.exe --
# they default off on any other compiler instead of failing the configure.
set(GTL_REAL_MSVC_CONDITIONS "MSVC" "NOT CMAKE_CXX_COMPILER_ID STREQUAL \"Clang\"")
cmake_dependent_option(GTL_BUILD_MFC "Build gtl.mfc (MFC extension DLL)" ON "${GTL_REAL_MSVC_CONDITIONS}" OFF)
cmake_dependent_option(GTL_BUILD_TEST_WIN "Build the 'test_win' MFC sample app" ON "${GTL_REAL_MSVC_CONDITIONS}" OFF)

# Qt is installed separately from vcpkg (aqt/offline installer), so hint CMake at the
# machine-local install if it exists. Override with -DCMAKE_PREFIX_PATH=... as needed.
set(GTL_QT_ROOT_HINT "D:/DevTools/Qt/6.8.3/msvc2022_64" CACHE PATH "Optional Qt6 install used to locate Qt6Config.cmake")
if(GTL_QT_ROOT_HINT AND EXISTS "${GTL_QT_ROOT_HINT}")
	list(APPEND CMAKE_PREFIX_PATH "${GTL_QT_ROOT_HINT}")
endif()

add_subdirectory(src/gtl)

if(GTL_BUILD_SHAPE)
	add_subdirectory(src/shape)
endif()
if(GTL_BUILD_WIN)
	add_subdirectory(src/win)
endif()
if(GTL_BUILD_MFC)
	add_subdirectory(src/mfc)
endif()
if(GTL_BUILD_WX)
	add_subdirectory(src/wx)
endif()
if(GTL_BUILD_QT)
	add_subdirectory(src/qt)
endif()

if(GTL_BUILD_TEST)
	add_subdirectory(src/test)
endif()
if(GTL_BUILD_BENCH)
	add_subdirectory(src/bench)
endif()
if(GTL_BUILD_TEST_SEQUENCE)
	add_subdirectory(src/test_sequence)
endif()
if(GTL_BUILD_QT AND GTL_BUILD_TEST_QT)
	add_subdirectory(src/test_qt)
endif()
if(GTL_BUILD_MFC AND GTL_BUILD_TEST_WIN)
	add_subdirectory(src/test_win)
endif()

# test_ui_webview2 and UnitTest are intentionally not ported: UnitTest uses the MSVC-only
# CppUnitTest framework and isn't even in gtl.sln, and test_ui_webview2 is a WebView2/ATL/
# COM sample, neither of which builds under MinGW/g++ (the eventual MSYS2 ucrt64 target)
# either way. They keep building via their own vcxproj as before. mfc/test_win above are
# also MSVC-only (no MFC on MinGW) but are now CMake targets, gated by GTL_BUILD_MFC.

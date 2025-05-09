@echo off
rem ======================================================
rem START
rem ======================================================

set VCPKG_FOLDER=%VCPKG_ROOT%
if "%VCPKG_FOLDER%"=="" set VCPKG_FOLDER=\vcpkg

mv %VCPKG_FOLDER%\installed\x64-windows\include\opencv4\opencv2 %VCPKG_FOLDER%\installed\x64-windows\include\opencv2

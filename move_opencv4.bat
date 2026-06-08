@echo off
rem ======================================================
rem START
rem ======================================================
setlocal
set VCPKG_INSTALLED=..\..\vcpkg_installed\
move "%VCPKG_INSTALLED%\x64-windows\include\opencv4\opencv2" "%VCPKG_INSTALLED%\x64-windows\include\opencv2"
endlocal
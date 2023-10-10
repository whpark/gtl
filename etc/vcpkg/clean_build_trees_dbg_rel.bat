@echo off
rem Save current folder
set SourceFolder=%~dp0

rem check if %VCPKG_ROOT% is empty or not
set VCPKG_FOLDER=%VCPKG_ROOT%
if "%VCPKG_FOLDER%"=="" (
	set VCPKG_FOLDER=\vcpkg
)

pushd "%VCPKG_FOLDER%\buildtrees"
echo Remove temp files?
echo     "%cd%"
pause

echo removing *-dbg
for /D /R %%1 IN (*-dbg) DO RD /S /Q "%%1"
echo removing *-rel
for /D /R %%1 IN (*-rel) DO RD /S /Q "%%1"
popd

set VCPKG_FOLDER=
pause

@echo off
goto start


rem ======================================================
rem Sub Routines
rem ======================================================
goto end

rem ======================================================
rem START
rem ======================================================
:start

rem Save current folder
set SourceFolder=%~dp0
set FAILED=
set PopDirectory=0
set CallBootStrap=0

rem check if %VCPKG_ROOT% is empty or not, replace '/' to '\'
set VCPKG_FOLDER=%VCPKG_ROOT:/=\%
if "%VCPKG_FOLDER%"=="" set VCPKG_FOLDER=\vcpkg

if exist "%VCPKG_FOLDER%" (
	echo Install?
	echo     "%VCPKG_FOLDER%"
	pause
) else (
	echo Clone a vcpkg ?
	echo     "%VCPKG_FOLDER%"
	pause
	git clone https://github.com/microsoft/vcpkg.git "%VCPKG_FOLDER%"
	rem if %errorlevel% neq 0 set FAILED=%FAILED% failed : cloning vcpkg (%ERRORLEVEL%) & goto end
	set CallBootStrap=1
)

rem 덮어쓰기 - x64-windows : v143 (MSVC2022)
xcopy /schry triplets %VCPKG_FOLDER%\triplets
pushd "%VCPKG_FOLDER%" & set PopDirectory=1
if %CallBootStrap% neq 0 (
	call bootstrap-vcpkg.bat
	vcpkg integrate install
)

rem x64
rem echo installing glaze:x64
rem vcpkg install --triplet=x64-windows --recurse glaze --overlay-ports=%SourceFolder%\ports\glaze\
rem if %errorlevel% neq 0 set FAILED=%FAILED% glaze:x64,

rem echo installing cppcoro:x64
rem vcpkg install --triplet=x64-windows --recurse cppcoro --overlay-ports=%SourceFolder%\ports\cppcoro-andreasbuhr\
rem if %errorlevel% neq 0 set FAILED=%FAILED% cppcoro:x64,

echo installing x64-windows...
vcpkg install --triplet=x64-windows --recurse gtest catch2 benchmark boost flux 7zip bzip2 libarchive fmt imgui spdlog scnlib ctre cpp-peglib ctpg foonathan-lexy ms-gsl magic-enum libenvpp winreg tinyxml2 icu utfcpp libiconv freetype cpr glaze nlohmann-json tomlplusplus opengl glew glm glfw3 freeglut tinyspline libxml2 libxmlmm openssl exprtk eigen3 libdmtx freeimage blend2d ffmpeg[all] aravis[introspection,packet-socket,usb] opencv[ade,aravis,calib3d,contrib,core,cuda,cudnn,dnn,dnn-cuda,eigen,ffmpeg,freetype,fs,gapi,gdcm,gstreamer,hdf,highgui,intrinsics,ipp,jpeg,jpegxl,openexr,opengl,openjpeg,openmp,png,python,quality,quirc,rgbd,sfm,text,thread,tiff,vtk,webp] vtk[atlmfc,opengl] wxwidgets[debug-support,example,fonts,media,webview] eventpp xlnt
rem deleted : maddy, websocketpp,
if %errorlevel% neq 0 set FAILED=%FAILED% x64-windows,

rem skip x86
goto end

rem x86
echo installing glaze:x86
vcpkg install --triplet=x86-windows --recurse glaze --overlay-ports=%SourceFolder%\ports\glaze\
if %errorlevel% neq 0 set FAILED=%FAILED% glaze:x86,

echo installing x86-windows...
vcpkg install --triplet=x86-windows --recurse gtest catch2 benchmark boost flux cppzmq grpc 7zip bzip2 libarchive fmt imgui spdlog scnlib ms-gsl magic-enum libenvpp winreg tinyxml2 nlohmann-json tomlplusplus libxml2 libxmlmm libmodbus openssl opencv[core] wxwidgets[debug-support,fonts]
if %errorlevel% neq 0 set FAILED=%FAILED% x86-windows,

rem ================================================
rem end
:end

rem PopDirectory
if %PopDirectory% neq 0 popd & set PopDirectory= & set CallBootStrap=

rem if vcpkg/
if Exist "%VCPKG_FOLDER%\installed\x64-windows\include\opencv4\opencv2" (
	echo !====
	echo !  move include/opencv4/opencv2 to include/opencv2/
	echo !====
	call move_opencv4.bat
) else (
	echo !====
	echo !  skip move include/opencv4/opencv2 to include/opencv2/
	echo !====
)

rem cleanup
set VCPKG_FOLDER=

if "%FAILED%" neq "" (
	echo
	echo !!======================================!!
	echo !!                                      !!
	echo !! FAILED:%FAILED%	!!
	echo !!                                      !!
	echo !!======================================!!
	pause
)

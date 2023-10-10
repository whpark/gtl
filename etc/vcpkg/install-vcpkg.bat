rem Save current folder
set SourceFolder=%~dp0

rem check if %VCPKG_ROOT% is empty or not
if "%VCPKG_ROOT%"=="" (
	set VCPKG_ROOT=\vcpkg
)
echo VCPKG_ROOT="%VCPKG_ROOT%"

rem check if vcpkg path exists, else clone it

if not exist "%VCPKG_ROOT%" (
	git clone https://github.com/microsoft/vcpkg.git "%VCPKG_ROOT%"
	pushd "%VCPKG_ROOT%"
	call bootstrap-vcpkg.bat
	popd
)

pushd "%VCPKG_ROOT%"

rem x64
vcpkg install --triplet=x64-windows --recurse glaze --overlay-ports=%SourceFolder%\ports\glaze\
vcpkg install --triplet=x64-windows --recurse gtest catch2 benchmark boost 7zip bzip2 fmt imgui spdlog scnlib ctre ms-gsl magic-enum libenvpp winreg tinyxml2 icu utfcpp libiconv freetype nngpp grpc cpr nlohmann-json tomlplusplus opengl glew glm glfw3 libxml2 libxmlmm libmodbus openssl eigen3 freeimage blend2d opencv[contrib,core,cuda,default-features,dnn,freetype,ipp,opengl,python,vtk] vtk[atlmfc,opengl] wxwidgets[debug-support,example,fonts,media,webview] eventpp

rem x86
vcpkg install --triplet=x86-windows --recurse glaze --overlay-ports=%SourceFolder%\ports\glaze\
vcpkg install --triplet=x86-windows --recurse gtest catch2 benchmark boost nngpp grpc 7zip bzip2 fmt imgui spdlog scnlib ms-gsl magic-enum libenvpp winreg tinyxml2 nlohmann-json tomlplusplus libxml2 libxmlmm libmodbus openssl opencv[core] wxwidgets[debug-support,fonts]

popd

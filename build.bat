@echo off
msbuild gtl.sln /p:Configuration=Debug
if ErrorLevel 1 goto errend
msbuild gtl.sln /p:Configuration=Release
if ErrorLevel 1 goto errend

goto end

:errend
@echo on
PAUSE ERROR --

:end

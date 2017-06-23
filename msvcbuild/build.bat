@echo off

set OLDCD=%CD%

call mkdir build
call cd build

call cmake -G "Visual Studio 14 2015" -T "v140_xp" ..\..

if errorlevel 1 (
	echo cmake generation failed, exiting
	goto fail
)

call cmake --build . --config Release

if errorlevel 1 (
	echo cmake build failed, exiting
	goto fail
)

echo.
echo.
echo Done! Built binaries are in build/bin/Release.
cd %OLDCD%
pause
exit /b

:fail
cd %OLDCD%
pause
exit /b 1
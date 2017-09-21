@echo off

:: Save the current directory so that we can cd back to it at the end of the script.

set OLDCD=%CD%

:: Create and enter the build directory, and delete build\CMakeCache.txt if it exists for a clean build.

if exist build (
	if exist build\CMakeCache.txt (
		call del build\CMakeCache.txt
	)
) else (
	call mkdir build
)

call cd build

:: Generate the Visual Studio project files.

call cmake -G "Visual Studio 15 2017" -T "v141_xp" -DMSSQL=1 ..\..

if errorlevel 1 (
	echo cmake generation failed, exiting
	goto fail
)

:: Build in release mode.

call cmake --build . --config RelWithDebInfo

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
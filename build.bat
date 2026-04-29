@echo off
setlocal

:: Try common Visual Studio install locations in order.
set VCVARS=
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)

if "%VCVARS%"=="" (
    echo ERROR: Could not locate vcvars64.bat. Install Visual Studio 2022 with C++ workload.
    exit /b 1
)

call "%VCVARS%" >nul 2>&1

if not exist build mkdir build

cl.exe /std:c++20 /EHsc /W4 /Zi ^
    /I include ^
    src\main.cpp src\find.cpp src\id3.cpp ^
    /Fe:build\mp3tag.exe ^
    /Fo:build\ ^
    /Fd:build\ ^
    /link kernel32.lib

if %errorlevel% neq 0 (
    echo.
    echo Build FAILED.
    exit /b %errorlevel%
)

echo.
echo Build succeeded: build\mp3tag.exe

@echo off
setlocal enabledelayedexpansion

:: ============================================================
:: build.bat – Build uc.launcher.exe with MSVC cl.exe
::
:: Usage:
::   build.bat [Release|Debug]   (default: Release)
::
:: Requirements:
::   Visual Studio Build Tools 2019/2022 (or full VS install)
::   containing cl.exe, rc.exe, link.exe for x64.
:: ============================================================

set CONFIG=%~1
if "%CONFIG%"=="" set CONFIG=Release

:: -------------------------------------------------------
:: Locate vcvarsall.bat via vswhere (ships with VS 15.2+)
:: -------------------------------------------------------
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% (
    echo [ERROR] vswhere.exe not found.
    echo         Install Visual Studio Build Tools 2019 or 2022 and try again.
    echo         Download: https://aka.ms/vs/17/release/vs_BuildTools.exe
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (
    `%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`
) do set VS_INSTALL=%%i

if "%VS_INSTALL%"=="" (
    echo [ERROR] No MSVC toolset found. Make sure the "C++ build tools" workload is installed.
    exit /b 1
)

set VCVARSALL="%VS_INSTALL%\VC\Auxiliary\Build\vcvarsall.bat"
if not exist %VCVARSALL% (
    echo [ERROR] vcvarsall.bat not found at: %VCVARSALL%
    exit /b 1
)

echo [INFO] Using VS installation: %VS_INSTALL%
call %VCVARSALL% x64 >nul

:: -------------------------------------------------------
:: Compiler / linker flags
:: -------------------------------------------------------
set CL_FLAGS=/nologo /std:c++17 /EHsc /W3
set LINK_FLAGS=/SUBSYSTEM:WINDOWS /NOLOGO

if /i "%CONFIG%"=="Debug" (
    set CL_FLAGS=%CL_FLAGS% /Od /Zi /D_DEBUG
    set LINK_FLAGS=%LINK_FLAGS% /DEBUG
) else (
    set CL_FLAGS=%CL_FLAGS% /O2 /DNDEBUG
)

:: -------------------------------------------------------
:: Compile resource file (rc.exe → .res)
:: -------------------------------------------------------
echo [INFO] Compiling resource file...
rc.exe /nologo /fo uc.launcher.res uc.launcher.rc
if errorlevel 1 (
    echo [ERROR] rc.exe failed.
    exit /b 1
)

:: -------------------------------------------------------
:: Compile + link
:: -------------------------------------------------------
echo [INFO] Compiling and linking (%CONFIG%)...
cl.exe %CL_FLAGS% uc.launcher.cpp uc.launcher.res ^
    /Fe:uc.launcher.exe ^
    /link %LINK_FLAGS% ^
    kernel32.lib user32.lib
if errorlevel 1 (
    echo [ERROR] Build failed.
    del /q uc.launcher.res 2>nul
    exit /b 1
)

:: -------------------------------------------------------
:: Clean up intermediate files
:: -------------------------------------------------------
del /q uc.launcher.res uc.launcher.obj 2>nul

echo [OK] Build succeeded: uc.launcher.exe  (config: %CONFIG%)
endlocal

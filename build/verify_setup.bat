@echo off
echo ==========================================
echo wxTimelineCtrl Build Setup Verification
echo ==========================================
echo.

REM Check CMake
echo Checking CMake...
cmake --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] CMake found
    cmake --version | findstr "cmake version"
) else (
    echo [ERROR] CMake not found or not in PATH
    set HAS_ERRORS=1
)
echo.

REM Check WXWIN environment variable
echo Checking WXWIN environment variable...
if defined WXWIN (
    echo [OK] WXWIN set to: %WXWIN%
    if exist "%WXWIN%" (
        echo [OK] WXWIN path exists
        if exist "%WXWIN%\include\wx\wx.h" (
            echo [OK] Found wx.h header file
        ) else (
            echo [ERROR] wx.h header file not found
            set HAS_ERRORS=1
        )
    ) else (
        echo [ERROR] WXWIN path does not exist: %WXWIN%
        set HAS_ERRORS=1
    )
) else (
    echo [ERROR] WXWIN environment variable not set
    echo Please set it to your wxWidgets installation directory
    echo Example: set WXWIN=C:\wxWidgets-3.2.4
    set HAS_ERRORS=1
)
echo.

REM Check library files
echo Checking library files...
if exist "..\lib\wxTimelineCtrl.h" (
    echo [OK] Found wxTimelineCtrl.h
) else (
    echo [ERROR] Missing lib\wxTimelineCtrl.h
    set HAS_ERRORS=1
)

if exist "..\lib\TimelineItemData.h" (
    echo [OK] Found TimelineItemData.h
) else (
    echo [ERROR] Missing lib\TimelineItemData.h
    set HAS_ERRORS=1
)

REM Check application files
echo Checking application files...
if exist "..\app\wxTimelineCtrlApp.cpp" (
    echo [OK] Found wxTimelineCtrlApp.cpp
) else (
    echo [ERROR] Missing app\wxTimelineCtrlApp.cpp
    set HAS_ERRORS=1
)

if exist "..\app\SampleData.h" (
    echo [OK] Found SampleData.h
) else (
    echo [ERROR] Missing app\SampleData.h
    set HAS_ERRORS=1
)
echo.

REM Check build directories
echo Checking build directories...
if exist "lib" (
    echo [OK] Found lib directory
) else (
    echo [ERROR] Missing lib directory
    set HAS_ERRORS=1
)

if exist "app" (
    echo [OK] Found app directory
) else (
    echo [ERROR] Missing app directory
    set HAS_ERRORS=1
)
echo.

echo ==========================================
echo Verification Summary
echo ==========================================
if defined HAS_ERRORS (
    echo [ERROR] Issues found. Please resolve them before building.
    echo.
    echo Common solutions:
    echo - Install CMake and add it to PATH
    echo - Set WXWIN environment variable to your wxWidgets directory
    echo - Ensure wxWidgets is properly built and installed
) else (
    echo [OK] All checks passed! Ready to build.
    echo.
    echo To build the project, run:
    echo   build.bat [Debug^|Release]
    echo.
    echo Or use PowerShell:
    echo   .\build.ps1 [-Configuration Debug^|Release]
)
echo.

pause

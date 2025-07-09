@echo off
REM Build script for wxTimelineCtrl project

echo ==========================================
echo wxTimelineCtrl Build Script
echo ==========================================

REM Check if WXWIN environment variable is set
if not defined WXWIN (
    echo ERROR: WXWIN environment variable is not set!
    echo Please set it to your wxWidgets installation directory.
    echo Example: set WXWIN=C:\wxWidgets-3.2.4
    pause
    exit /b 1
)

echo Using wxWidgets from: %WXWIN%

REM Create build directory if it doesn't exist
if not exist "generated" mkdir generated
cd generated

REM Configuration to build (default is Release)
set BUILD_CONFIG=%1
if "%BUILD_CONFIG%"=="" set BUILD_CONFIG=Release

echo Building configuration: %BUILD_CONFIG%

REM Generate build files
echo Generating CMake build files...
cmake -G "Visual Studio 17 2022" -A x64 ..
if %errorlevel% neq 0 (
    echo ERROR: CMake generation failed!
    pause
    exit /b %errorlevel%
)

REM Build the project
echo Building project...
cmake --build . --config %BUILD_CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b %errorlevel%
)

echo.
echo ==========================================
echo Build completed successfully!
echo ==========================================
echo.
echo Executable location: %CD%\%BUILD_CONFIG%\wxTimelineCtrlTest.exe
echo.
echo To run the application:
echo   cd %BUILD_CONFIG%
echo   wxTimelineCtrlTest.exe
echo.

pause

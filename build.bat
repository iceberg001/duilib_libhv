@echo off
chcp 65001 >nul
echo ========================================
echo   NetAPI Debugger - Build Script
echo ========================================
echo.

set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build"

echo [1/4] Setting up 3rdparty dependencies...
powershell -ExecutionPolicy Bypass -File "%PROJECT_DIR%setup_3rdparty.ps1"
if errorlevel 1 (
    echo ERROR: 3rdparty setup failed!
    exit /b 1
)
echo.

echo [2/4] Configuring CMake...
cmake -B "%BUILD_DIR%" -S "%PROJECT_DIR%." -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo ERROR: CMake configuration failed!
    exit /b 1
)
echo.

echo [3/4] Building Release x64...
cmake --build "%BUILD_DIR%" --config Release --parallel
if errorlevel 1 (
    echo ERROR: Build failed!
    exit /b 1
)
echo.

echo [4/4] Build complete!
echo   Output: %BUILD_DIR%\Release\net_api_test.exe
echo.
dir "%BUILD_DIR%\Release\net_api_test.exe"
echo.
echo You can copy net_api_test.exe to any location and run it standalone.

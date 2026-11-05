@echo off
chcp 65001 >nul
echo ========================================
echo   Setup 3rdparty dependencies
echo ========================================

set DUI_SRC=I:\跨平台\duilib\duilib
set SDK_SRC=J:\MyGameServer\MyNetSDK\sdk_output
set DST=J:\net_api_test\3rdparty

echo.
echo [1/4] Copying duilib headers...
if not exist "%DST%\duilib\include" mkdir "%DST%\duilib\include"
xcopy /Y /S /Q "%DUI_SRC%\include\*" "%DST%\duilib\include\" >nul

echo [2/4] Copying duilib libraries...
if not exist "%DST%\duilib\lib\Debug" mkdir "%DST%\duilib\lib\Debug"
if not exist "%DST%\duilib\lib\Release" mkdir "%DST%\duilib\lib\Release"
copy /Y "%DUI_SRC%\build_win\Debug\DuiLib.lib" "%DST%\duilib\lib\Debug\" >nul
copy /Y "%DUI_SRC%\build_win\Release\DuiLib.lib" "%DST%\duilib\lib\Release\" >nul
copy /Y "%DUI_SRC%\build_win\third-party\zlib-1.3.1\Debug\zlibstaticd.lib" "%DST%\duilib\lib\Debug\" >nul
copy /Y "%DUI_SRC%\build_win\third-party\zlib-1.3.1\Release\zlibstatic.lib" "%DST%\duilib\lib\Release\" >nul

echo [3/4] Copying libhv headers...
if not exist "%DST%\libhv\include" mkdir "%DST%\libhv\include"
xcopy /Y /S /Q "%SDK_SRC%\include\*" "%DST%\libhv\include\" >nul

echo [4/4] Copying libhv libraries...
if not exist "%DST%\libhv\lib\Debug" mkdir "%DST%\libhv\lib\Debug"
if not exist "%DST%\libhv\lib\Release" mkdir "%DST%\libhv\lib\Release"
copy /Y "%SDK_SRC%\lib\x64\Debug\hv_static.lib" "%DST%\libhv\lib\Debug\" >nul
copy /Y "%SDK_SRC%\lib\x64\Release\hv_static.lib" "%DST%\libhv\lib\Release\" >nul

echo.
echo Done! 3rdparty directory is ready.
echo Directory structure:
echo   3rdparty\duilib\include\   - duilib headers
echo   3rdparty\duilib\lib\Debug\ - DuiLib.lib, zlibstaticd.lib
echo   3rdparty\duilib\lib\Release\ - DuiLib.lib, zlibstatic.lib
echo   3rdparty\libhv\include\   - libhv headers
echo   3rdparty\libhv\lib\Debug\ - hv_static.lib
echo   3rdparty\libhv\lib\Release\ - hv_static.lib

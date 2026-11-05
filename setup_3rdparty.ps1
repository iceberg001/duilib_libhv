$ErrorActionPreference = 'Stop'

$DUI_SRC = 'I:\跨平台\duilib\duilib'
$SDK_SRC = 'J:\MyGameServer\MyNetSDK\sdk_output'
$DST = 'J:\net_api_test\3rdparty'

# Create directories
$dirs = @(
    "$DST\duilib\include",
    "$DST\duilib\lib\Debug",
    "$DST\duilib\lib\Release",
    "$DST\libhv\include",
    "$DST\libhv\lib\Debug",
    "$DST\libhv\lib\Release"
)
foreach ($d in $dirs) {
    New-Item -ItemType Directory -Force -Path $d | Out-Null
}

# Copy duilib headers
Copy-Item -Path "$DUI_SRC\include\*" -Destination "$DST\duilib\include\" -Recurse -Force
$count1 = (Get-ChildItem "$DST\duilib\include" -File).Count
Write-Output "[1] duilib headers: $count1 files"

# Copy duilib libs
Copy-Item "$DUI_SRC\build_win\Debug\DuiLib.lib" "$DST\duilib\lib\Debug\" -Force
Copy-Item "$DUI_SRC\build_win\Release\DuiLib.lib" "$DST\duilib\lib\Release\" -Force
Copy-Item "$DUI_SRC\build_win\third-party\zlib-1.3.1\Debug\zlibstaticd.lib" "$DST\duilib\lib\Debug\" -Force
Copy-Item "$DUI_SRC\build_win\third-party\zlib-1.3.1\Release\zlibstatic.lib" "$DST\duilib\lib\Release\" -Force
Write-Output "[2] duilib libs copied"

# Copy libhv headers
Copy-Item -Path "$SDK_SRC\include\*" -Destination "$DST\libhv\include\" -Recurse -Force
$count2 = (Get-ChildItem "$DST\libhv\include" -Recurse -File).Count
Write-Output "[3] libhv headers: $count2 files"

# Copy libhv libs
Copy-Item "$SDK_SRC\lib\x64\Debug\hv_static.lib" "$DST\libhv\lib\Debug\" -Force
Copy-Item "$SDK_SRC\lib\x64\Release\hv_static.lib" "$DST\libhv\lib\Release\" -Force
Write-Output "[4] libhv libs copied"

Write-Output "Done! 3rdparty setup complete."

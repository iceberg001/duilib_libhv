Add-Type -AssemblyName System.Drawing
Get-ChildItem 'J:\net_api_test\res\image\*.png' | ForEach-Object {
    $img = [System.Drawing.Image]::FromFile($_.FullName)
    Write-Output ("{0}: {1}x{2}" -f $_.Name, $img.Width, $img.Height)
    $img.Dispose()
}
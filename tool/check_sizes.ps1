Get-ChildItem 'J:\net_api_test\res\image\*.png' | ForEach-Object {
    $sizeKB = [math]::Round($_.Length / 1024, 1)
    Write-Output ("{0}: {1} KB" -f $_.Name, $sizeKB)
}
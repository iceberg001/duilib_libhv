Add-Type -AssemblyName System.Drawing
$imgs = @('vscrollbar.png', 'hscrollbar.png')
foreach ($name in $imgs) {
    $path = "J:\net_api_test\res\image\$name"
    $img = [System.Drawing.Image]::FromFile($path)
    Write-Host "$name : $($img.Width) x $($img.Height)"
    $img.Dispose()
}
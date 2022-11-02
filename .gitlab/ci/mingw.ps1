$erroractionpreference = "stop"

if ("$env:CMAKE_CONFIGURATION".Contains("mingw_osdn_io")) {
    $filename = "mingw.osdn.io-2022-10-03"
    $sha256sum = "4DCB8C351D8D855F7D3DFC2863A235042BF3DB6E69EA0BAE51FF9378189345CD"
} else {
    throw ('unknown CMAKE_CONFIGURATION: ' + "$env:CMAKE_CONFIGURATION")
}
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
Invoke-WebRequest -Uri "https://cmake.org/files/dependencies/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir")
Move-Item -Path "$outdir\$filename" -Destination "$outdir\mingw"
Remove-Item "$outdir\$tarball"

"$outdir/mingw /mingw" -replace '\\', '/' | Out-File -FilePath "$outdir\mingw\msys\1.0\etc\fstab" -Encoding ASCII

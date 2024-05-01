$erroractionpreference = "stop"

if ("$env:CMAKE_CONFIGURATION".Contains("openwatcom1.9")) {
    # Open Watcom 1.9
    # https://web.archive.org/web/20210312132437/http://www.openwatcom.org/download.php
    $filename = "open-watcom-1.9-1"
    $sha256sum = "FFE6F5BBA200912697C6EC26C4D3B2623A0358FBE7CBB23BD264CBF7D54E4988"
} else {
    throw ('unknown CMAKE_CONFIGURATION: ' + "$env:CMAKE_CONFIGURATION")
}
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
# This URL is only visible inside of Kitware's network.  See above filename table.
Invoke-WebRequest -Uri "https://cmake.org/files/dependencies/internal/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir")
Move-Item -Path "$outdir\$filename" -Destination "$outdir\watcom"

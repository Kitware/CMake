$erroractionpreference = "stop"

if ("$env:CMAKE_CONFIGURATION".Contains("msvc_v71")) {
    # MSVC v71 Toolset from Visual Studio 7 .NET 2003
    $filename = "msvc-v71-1"
    $sha256sum = "01637CDC670EA5D631E169E286ACDD1913A124E3C5AF4C3DFB37657ADE8BBA9F"
    $vcvars = "Vc7\bin\vcvars32.bat"
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
Move-Item -Path "$outdir\$filename" -Destination "$outdir\msvc"

$bat = Get-Content -path "$outdir\msvc\$vcvars.in" -Raw
$bat = $bat -replace "@VS_ROOT@","$outdir\msvc"
$bat | Set-Content -path "$outdir\msvc\$vcvars"

Set-Item -Force -Path "env:VCVARSALL" -Value "$outdir\msvc\$vcvars"

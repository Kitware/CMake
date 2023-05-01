$erroractionpreference = "stop"

if ("$env:CMAKE_CI_BUILD_NAME" -match "(^|_)(oneapi2023\.1\.0|intel2021\.9\.0)(_|$)") {
    # Intel oneAPI 2023.1.0
    $version = "2023.1.0"
    $filename = "intel-oneapi-$version-windows-1"
    $sha256sum = "5AFCA9E0B03894565209B1295476163ABEBB1F1388E0F3EF5B4D0F9189E65BDC"
} else {
    throw ('unknown CMAKE_CI_BUILD_NAME: ' + "$env:CMAKE_CI_BUILD_NAME")
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
Move-Item -Path "$outdir\$filename" -Destination "$outdir\intel"
Remove-Item "$outdir\$tarball"

$compiler = "$outdir\intel\compiler"
$bin = "$compiler\$version\windows\bin"
$null = New-Item -ItemType Junction -Path "$compiler\latest"   -Target "$compiler\$version"
$null = New-Item -ItemType HardLink -Path "$bin\icx-cl.exe"    -Target "$bin\icx.exe"
$null = New-Item -ItemType HardLink -Path "$bin\icx-cc.exe"    -Target "$bin\icx.exe"
$null = New-Item -ItemType HardLink -Path "$bin\icpx.exe"      -Target "$bin\icx.exe"
$bin = "$compiler\$version\windows\bin-llvm"
$null = New-Item -ItemType HardLink -Path "$bin\clang-cl.exe"  -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\clang-cpp.exe" -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\clang++.exe"   -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\lld-link.exe"  -Target "$bin\lld.exe"
$null = New-Item -ItemType HardLink -Path "$bin\ld.lld.exe"    -Target "$bin\lld.exe"
$null = New-Item -ItemType HardLink -Path "$bin\llvm-lib.exe"  -Target "$bin\llvm-ar.exe"
Clear-Variable -Name bin
Clear-Variable -Name compiler

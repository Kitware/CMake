$erroractionpreference = "stop"

if ("$env:CMAKE_CI_BUILD_NAME" -match "(^|_)(oneapi2024\.1\.0)(_|$)") {
    # Intel oneAPI 2024.1.0
    $version = "2024.1.0"
    $version_dir = "2024.1"
    $bin_dir = "bin"
    $llvm_dir = "bin\compiler"
    $filename = "intel-oneapi-$version-windows-1"
    $sha256sum = "CB6857C08CD815722913358EC495AA575C2EED730646098A6CF7178E55FA7323"
} elseif ("$env:CMAKE_CI_BUILD_NAME" -match "(^|_)(intel2021\.9\.0)(_|$)") {
    # Intel oneAPI 2023.1.0
    $version = "2023.1.0"
    $version_dir = "2023.1.0"
    $bin_dir = "windows\bin"
    $llvm_dir = "windows\bin-llvm"
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
$bin = "$compiler\$version_dir\$bin_dir"
$null = New-Item -ItemType Junction -Path "$compiler\latest"   -Target "$compiler\$version_dir"
$null = New-Item -ItemType HardLink -Path "$bin\icx-cl.exe"    -Target "$bin\icx.exe"
$null = New-Item -ItemType HardLink -Path "$bin\icx-cc.exe"    -Target "$bin\icx.exe"
$null = New-Item -ItemType HardLink -Path "$bin\icpx.exe"      -Target "$bin\icx.exe"
$bin = "$compiler\$version_dir\$llvm_dir"
$null = New-Item -ItemType HardLink -Path "$bin\clang-cl.exe"  -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\clang-cpp.exe" -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\clang++.exe"   -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\lld-link.exe"  -Target "$bin\lld.exe"
$null = New-Item -ItemType HardLink -Path "$bin\ld.lld.exe"    -Target "$bin\lld.exe"
$null = New-Item -ItemType HardLink -Path "$bin\llvm-lib.exe"  -Target "$bin\llvm-ar.exe"
Clear-Variable -Name bin
Clear-Variable -Name compiler

Clear-Variable -Name llvm_dir
Clear-Variable -Name bin_dir
Clear-Variable -Name version_dir
Clear-Variable -Name version

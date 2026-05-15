$erroractionpreference = "stop"

if ("$env:CMAKE_CI_BUILD_NAME".Contains("clang22.1")) {
    # LLVM/Clang 22.1.5
    # https://github.com/llvm/llvm-project/releases/tag/llvmorg-22.1.5
    $filename = "llvm-22.1.5-win-x86_64-1"
    $sha256sum = "6230FBFBFB9B5B9E148A716787BCB384CB5D6042DDF5C219BFADD1AC20CB0A3F"
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
Move-Item -Path "$outdir\$filename" -Destination "$outdir\llvm"
Remove-Item "$outdir\$tarball"

$bin = "$outdir\llvm\bin"
$lib = "$outdir\llvm\lib"
$null = New-Item -ItemType HardLink -Path "$bin\clang++.exe"      -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\clang-cl.exe"     -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\clang-cpp.exe"    -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\flang-new.exe"    -Target "$bin\flang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\ld.lld.exe"       -Target "$bin\lld.exe"
$null = New-Item -ItemType HardLink -Path "$bin\ld64.lld.exe"     -Target "$bin\lld.exe"
$null = New-Item -ItemType HardLink -Path "$bin\lld-link.exe"     -Target "$bin\lld.exe"
$null = New-Item -ItemType HardLink -Path "$bin\llvm-lib.exe"     -Target "$bin\llvm-ar.exe"
$null = New-Item -ItemType HardLink -Path "$bin\llvm-ranlib.exe"  -Target "$bin\llvm-ar.exe"
$null = New-Item -ItemType HardLink -Path "$bin\llvm-objcopy.exe" -Target "$bin\llvm-strip.exe"
$null = New-Item -ItemType HardLink -Path "$bin\libiomp5md.dll"   -Target "$bin\libomp.dll"
$null = New-Item -ItemType HardLink -Path "$lib\libiomp5md.lib"   -Target "$lib\libomp.lib"
Clear-Variable -Name bin
Clear-Variable -Name lib

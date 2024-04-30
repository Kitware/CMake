$erroractionpreference = "stop"

$version = "5.9.2"
$sha256sum = "8C053108528EB2DAD84C33D6F0834A3A1444D21BA1A89D591AB149304A62F6B5"
$filename = "swift-$version-win-x86_64-1"
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
Move-Item -Path "$outdir\$filename" -Destination "$outdir\swift"
Remove-Item "$outdir\$tarball"

$bin = "$outdir\swift\Toolchains\unknown-Asserts-development.xctoolchain\usr\bin"
$null = New-Item -ItemType HardLink -Path "$bin\clang++.exe"      -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\clang-cl.exe"     -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\clang-cpp.exe"    -Target "$bin\clang.exe"
$null = New-Item -ItemType HardLink -Path "$bin\ld.lld.exe"       -Target "$bin\lld.exe"
$null = New-Item -ItemType HardLink -Path "$bin\ld64.lld.exe"     -Target "$bin\lld.exe"
$null = New-Item -ItemType HardLink -Path "$bin\lld-link.exe"     -Target "$bin\lld.exe"
$null = New-Item -ItemType HardLink -Path "$bin\llvm-dlltool.exe" -Target "$bin\llvm-ar.exe"
$null = New-Item -ItemType HardLink -Path "$bin\llvm-lib.exe"     -Target "$bin\llvm-ar.exe"
$null = New-Item -ItemType HardLink -Path "$bin\llvm-ranlib.exe"  -Target "$bin\llvm-ar.exe"
$null = New-Item -ItemType HardLink -Path "$bin\llvm-objcopy.exe" -Target "$bin\llvm-strip.exe"
$null = New-Item -ItemType HardLink -Path "$bin\swiftc.exe"       -Target "$bin\swift.exe"
$null = New-Item -ItemType HardLink -Path "$bin\swift-api-digester.exe"        -Target "$bin\swift-frontend.exe"
$null = New-Item -ItemType HardLink -Path "$bin\swift-autolink-extract.exe"    -Target "$bin\swift-frontend.exe"
$null = New-Item -ItemType HardLink -Path "$bin\swift-symbolgraph-extract.exe" -Target "$bin\swift-frontend.exe"
Clear-Variable -Name bin

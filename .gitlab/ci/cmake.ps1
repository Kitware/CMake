$erroractionpreference = "stop"

$version = "3.30.1"

if ("$env:PROCESSOR_ARCHITECTURE" -eq "AMD64") {
    $sha256sum = "CF7788FF9D92812DA194847D4EC874FC576F34079987D0F20C96CD09E2A16220"
    $platform = "windows-x86_64"
} elseif ("$env:PROCESSOR_ARCHITECTURE" -eq "ARM64") {
    $sha256sum = "02B433F70AA549449BE2D53046D0179590BF3B6290D9FDA3FBBB23F96A4F2802"
    $platform = "windows-arm64"
} else {
    throw ('unknown PROCESSOR_ARCHITECTURE: ' + "$env:PROCESSOR_ARCHITECTURE")
}

$filename = "cmake-$version-$platform"
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
Invoke-WebRequest -Uri "https://github.com/Kitware/CMake/releases/download/v$version/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir")
Move-Item -Path "$outdir\$filename" -Destination "$outdir\cmake"

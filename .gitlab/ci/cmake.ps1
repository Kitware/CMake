$erroractionpreference = "stop"

$version = "3.30.0-rc3"

if ("$env:PROCESSOR_ARCHITECTURE" -eq "AMD64") {
    $sha256sum = "ECAE920994E96649A22C0DF093718537354AF08054C998C8F4B862040AEBEE87"
    $platform = "windows-x86_64"
} elseif ("$env:PROCESSOR_ARCHITECTURE" -eq "ARM64") {
    $sha256sum = "9768BE435BB5619C7539FE66F180F0056A59F29BDC28CAA3E5A0FD1EFE963A93"
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

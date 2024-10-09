$erroractionpreference = "stop"

$version = "3.11.3"

if ("$env:PROCESSOR_ARCHITECTURE" -eq "AMD64") {
    $sha256sum = "7419B2E98516FBD0B66A1237B80187FFB21D32E47B4A4235C2D9D6379597070F"
    $arch = "amd64"
} elseif ("$env:PROCESSOR_ARCHITECTURE" -eq "ARM64") {
    $sha256sum = "03BAD6A7C898FC8F693982437AAB6DB698107B82EA93F76424195AE2C161246C"
    $arch = "arm64"
} else {
    throw ('unknown PROCESSOR_ARCHITECTURE: ' + "$env:PROCESSOR_ARCHITECTURE")
}

$filename = "python-$version-embed-$arch"
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
Invoke-WebRequest -Uri "https://cmake.org/files/dependencies/internal/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir\python3")
Remove-Item "$outdir\python3\*._pth" # Avoid sys.path specific to embedded python.
Remove-Item "$outdir\$tarball"

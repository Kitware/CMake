$erroractionpreference = "stop"

if ("$env:CMAKE_CI_JOB_NIGHTLY_NINJA" -eq "true" -And "$env:CMAKE_CI_NIGHTLY" -eq "true") {
    & .gitlab/ci/ninja-nightly.ps1
    exit $LASTEXITCODE
}

$version = "1.12.1"

if ("$env:PROCESSOR_ARCHITECTURE" -eq "AMD64") {
    $sha256sum = "F550FEC705B6D6FF58F2DB3C374C2277A37691678D6ABA463ADCBB129108467A"
    $filename = "ninja-win"
} elseif ("$env:PROCESSOR_ARCHITECTURE" -eq "ARM64") {
    $sha256sum = "79C96A50E0DEAFEC212CFA85AA57C6B74003F52D9D1673DDCD1EAB1C958C5900"
    $filename = "ninja-winarm64"
} else {
    throw ('unknown PROCESSOR_ARCHITECTURE: ' + "$env:PROCESSOR_ARCHITECTURE")
}

$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
Invoke-WebRequest -Uri "https://github.com/ninja-build/ninja/releases/download/v$version/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir")

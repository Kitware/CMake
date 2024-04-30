$erroractionpreference = "stop"

if ("$env:CMAKE_CI_JOB_NIGHTLY_NINJA" -eq "true" -And "$env:CMAKE_CI_NIGHTLY" -eq "true") {
    & .gitlab/ci/ninja-nightly.ps1
    exit $LASTEXITCODE
}

$version = "1.11.0"
$sha256sum = "D0EE3DA143211AA447E750085876C9B9D7BCDD637AB5B2C5B41349C617F22F3B"
$filename = "ninja-win"
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

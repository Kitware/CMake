$erroractionpreference = "stop"

$sha256sum = "128FDD846FE24F8594EED37D1D8929A0EA78DF563537C0C1B1861A635013FFF8"
$tarball = "unstable-jom-2018-12-12.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
Invoke-WebRequest -Uri "https://cmake.org/files/dependencies/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Expand-Archive -Path "$outdir\$tarball" -DestinationPath "$outdir\jom"

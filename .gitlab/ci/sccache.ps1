$erroractionpreference = "stop"

# 0.2.13 is unavailable right now.
# https://github.com/mozilla/sccache/issues/677
$version = "0.2.12"
$sha256sum = "FD05E91C59B9497D4EBAE311B47A982F2A6EB942DCA3C9C314CC1FB36F8BC64D"
$filename = "sccache-$version-x86_64-pc-windows-msvc"
$tarball = "$filename.tar.gz"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
Invoke-WebRequest -Uri "https://github.com/mozilla/sccache/releases/download/$version/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

$curdir = $pwd.Path
Set-Location -Path "$outdir"
cmake -E tar xzf "$outdir\$tarball"
Move-Item -Path "$outdir\$filename\sccache.exe" -Destination "$outdir\sccache.exe"
Set-Location -Path "$curdir"

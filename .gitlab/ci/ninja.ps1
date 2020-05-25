$erroractionpreference = "stop"

$version = "1.10.0"
$sha256sum = "919FD158C16BF135E8A850BB4046EC1CE28A7439EE08B977CD0B7F6B3463D178"
$filename = "ninja-win"
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
Invoke-WebRequest -Uri "https://github.com/ninja-build/ninja/releases/download/v$version/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir")

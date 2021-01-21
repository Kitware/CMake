$erroractionpreference = "stop"

$release = "wix3112rtm"
$sha256sum = "2C1888D5D1DBA377FC7FA14444CF556963747FF9A0A289A3599CF09DA03B9E2E"
$filename = "wix311-binaries"
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
Invoke-WebRequest -Uri "https://github.com/wixtoolset/wix3/releases/download/$release/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir\wix\bin")

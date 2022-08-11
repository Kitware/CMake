$erroractionpreference = "stop"

$release = "v3.14.0.6526"
$sha256sum = "4C89898DF3BCAB13E12F7CA54399C35AD273475AD2CB6284611D00AE2D063C2C"
$filename = "wix-3.14.0.6526-win-i386"
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
#Invoke-WebRequest -Uri "https://wixtoolset.org/downloads/$release/$tarball" -OutFile "$outdir\$tarball"
Invoke-WebRequest -Uri "https://cmake.org/files/dependencies/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir\wix\bin")

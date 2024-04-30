$erroractionpreference = "stop"

$release = "wix314rtm"
$sha256sum = "13F067F38969FAF163D93A804B48EA0576790A202C8F10291F2000F0E356E934"
#$filename = "wix314-binaries"
$filename = "wix-3.14.0.8606-win-i386"
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
#Invoke-WebRequest -Uri "https://github.com/wixtoolset/wix3/releases/download/$release/$tarball" -OutFile "$outdir\$tarball"
Invoke-WebRequest -Uri "https://cmake.org/files/dependencies/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir\wix3")
Remove-Item "$outdir\$tarball"

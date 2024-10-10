$erroractionpreference = "stop"

$version = "4.0.4"
$sha256sum = "FB6E94C89B12FB65D3AA0CF9E9C630DAFCC7D57F1E66C7D6035CAD37A38CC284"
$filename = "wix-$version-win-any-1"
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
Invoke-WebRequest -Uri "https://cmake.org/files/dependencies/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir")
Move-Item -Path "$outdir\$filename" -Destination "$outdir\wix4"
Remove-Item "$outdir\$tarball"

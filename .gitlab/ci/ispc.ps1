$erroractionpreference = "stop"

$version = "1.18.0"
$sha256sum = "9210BB2D9D3711367FACCB37ACF49966696132560B565471C1C6121F4924A17E"
$filename = "ispc-v$version-windows"
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
Invoke-WebRequest -Uri "https://github.com/ispc/ispc/releases/download/v$version/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir")
Move-Item -Path "$outdir\$filename" -Destination "$outdir\ispc"

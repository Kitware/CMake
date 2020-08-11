$erroractionpreference = "stop"

$version = "3.17.2"
$sha256sum = "CF82B1EB20B6FBE583487656FCD496490FFCCDFBCBBA0F26E19F1C9C63B0B041"
$filename = "cmake-$version-win64-x64"
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
Invoke-WebRequest -Uri "https://github.com/Kitware/CMake/releases/download/v$version/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir")
Move-Item -Path "$outdir\$filename" -Destination "$outdir\cmake"

$erroractionpreference = "stop"

if ("$env:CMAKE_CONFIGURATION".Contains("borland5.5")) {
    # Borland C++ 5.5 Free Command-line Tools
    # https://web.archive.org/web/20110402064356/https://www.embarcadero.com/products/cbuilder/free-compiler
    $filename = "bcc5.5-1"
    $sha256sum = "895B76F8F1AD8030F31ACE239EBC623DC7379C121A540F55F611B93F3CB9AF52"
} elseif ("$env:CMAKE_CONFIGURATION".Contains("borland5.8")) {
    # Borland C++ Builder 2006
    # https://web.archive.org/web/20060303030019/https://www.borland.com/us/products/cbuilder/index.html
    $filename = "bcc5.8-1"
    $sha256sum = "C30981BFD540C933E76D82D873DEE05E7482F34F68E309065DE0D181C95F77E3"
} else {
    throw ('unknown CMAKE_CONFIGURATION: ' + "$env:CMAKE_CONFIGURATION")
}
$tarball = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
# This URL is only visible inside of Kitware's network.  See above filename table.
Invoke-WebRequest -Uri "https://cmake.org/files/dependencies/internal/$tarball" -OutFile "$outdir\$tarball"
$hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$outdir")
Move-Item -Path "$outdir\$filename" -Destination "$outdir\bcc"

$tools = "bcc32", "ilink32"
foreach ($tool in $tools) {
    $cfg = Get-Content -path "$outdir\bcc\bin\$tool.cfg.in" -Raw
    $cfg = $cfg -replace "@BCC_ROOT@","$outdir\bcc"
    $cfg | Set-Content -path "$outdir\bcc\bin\$tool.cfg"
}

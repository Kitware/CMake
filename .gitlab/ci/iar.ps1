$erroractionpreference = "stop"
$ProgressPreference = 'SilentlyContinue'
Add-Type -AssemblyName System.IO.Compression.FileSystem

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$iar_dir = New-Item -Force -ItemType Directory -Path "$outdir\iar"

$files = @{
    "bxarm-9.70.1.13552-1.zip" = "866792FA6881C28610558E87EE02A45752CC06E7550FB57682720CDC300DA0B7"
    "BXAVR-8102-1.zip"         = "862EFD23531854506070D5647F9B32197B80E5A727304BFBD8E386A3DAADF093"
    "BXRH850-3102-1.zip"       = "8D1D009A0D138C7CA8431316123CB85CE1B41319A68B997F90D2E338CD469C7F"
    "BXRISCV-3401-1.zip"       = "633F9BF64429923B0C478FB8ED0C47B3A67BC9B23DD9A2851DE66AC70DA64E06"
    "bxrl78-5.20.1.2826-1.zip" = "71E981EC18C5BC031A356D89C4D6579DF0B0EF9EB34AB3B46A48DA3F3737C2D4"
    "bxrx-5.20.1.6541-1.zip"   = "C000B333618A253482DCCB8F06CB902F6D9D0B7D554B863175D316004FBFFC82"
}

foreach ($f in $files.GetEnumerator()) {
    $tarball = $f.Name

    # This URL is only visible inside of Kitware's network.
    Invoke-WebRequest -Uri "https://cmake.org/files/dependencies/internal/iar/$tarball" -OutFile "$outdir\$tarball"
    $hash = Get-FileHash "$outdir\$tarball" -Algorithm SHA256
    if ($hash.Hash -ne $f.Value) {
        exit 1
    }

    [System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$tarball", "$iar_dir")
    Remove-Item "$outdir\$tarball"
}

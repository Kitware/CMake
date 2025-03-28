$erroractionpreference = "stop"
$ProgressPreference = 'SilentlyContinue'
Add-Type -AssemblyName System.IO.Compression.FileSystem

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$iar_dir = New-Item -Force -ItemType Directory -Path "$outdir\iar"

$files = @{
    "bxarm-9.50.2.71951-1.zip" = "8A1C16673CEDF95DB94214159EADF06E86E66FA177149D0F379AACA88E26BC15"
    "BXAVR-8102-1.zip"         = "862EFD23531854506070D5647F9B32197B80E5A727304BFBD8E386A3DAADF093"
    "BXRH850-3102-1.zip"       = "8D1D009A0D138C7CA8431316123CB85CE1B41319A68B997F90D2E338CD469C7F"
    "BXRISCV-3301-1.zip"       = "59FF23F7B98EE72567A23942DE799AF137791A19BFEC102B2A59821FABBCA55A"
    "BXRL78-5103-1.zip"        = "00398E7197735A7B0A4310BF906808E883548814475C12D6EF2C03388F77E6A7"
    "BXRX-5101-1.zip"          = "D63E95ECD454B4998946C2D9DC1CB6CEF69CE15524C11A123263E6A8E88D9899"
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

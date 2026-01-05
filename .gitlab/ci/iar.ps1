$erroractionpreference = "stop"
$ProgressPreference = 'SilentlyContinue'
Add-Type -AssemblyName System.IO.Compression.FileSystem

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$iar_dir = New-Item -Force -ItemType Directory -Path "$outdir\iar"

$files = @{
    "bxarm-9.70.2.18199-1.zip"  = "47F91EB1829754440ED57B112D068345F1662309748AD5F1F8A6F6DA7362A54E"
    "BXAVR-8103-1.zip"          = "53DA32EC4CF0CD0A2D4B81EF2CB5B8AF4D48B14A65661CB112394C634F897C8C"
    "bxrh850-3.20.1.2142-1.zip" = "21EBFC9B61F5B25DF91F6578C6311AABC5BEB9FC9A71CDC54B85727149079ACF"
    "BXRISCV-3402-1.zip"        = "6A694D111DED058944F55545F7E64E5B67DEC3E28B30B4436BEF6C0080C90F8D"
    "bxrl78-5.20.2.2949-1.zip"  = "81ED60CFB7A8528490C1CBFCB22894E56B0A32A401F2F79ADFEA1D626EE7AF34"
    "bxrx-5.20.1.6541-1.zip"    = "C000B333618A253482DCCB8F06CB902F6D9D0B7D554B863175D316004FBFFC82"
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

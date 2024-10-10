$erroractionpreference = "stop"

if ("$env:CMAKE_CONFIGURATION".Contains("orangec6.73.1")) {
    # OrangeC 6.73.1
    $archive = "ZippedBinaries6738.zip"
    $release = "Orange-C-v6.73.1"
    $sha256sum = "29BC506AB105B2BF1002129C37826B2153DF1C8D0F22B9A2C38ACA3FB72A5B89"
} else {
    throw ('unknown CMAKE_CONFIGURATION: ' + "$env:CMAKE_CONFIGURATION")
}

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
Invoke-WebRequest -Uri "https://github.com/LADSoft/OrangeC/releases/download/$release/$archive" -OutFile "$outdir\$archive"
$hash = Get-FileHash "$outdir\$archive" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$archive", "$outdir")
# The archive contains directory 'orangec', placed at '$outdir\orangec'.
Remove-Item "$outdir\$archive"

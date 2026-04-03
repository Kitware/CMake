$erroractionpreference = "stop"

if ("$env:CMAKE_CONFIGURATION".Contains("pellesc13.01")) {
    # PellesC 13.01
    # https://www.pellesc.se/1300/setup.exe
    $filename = "pellesc-13.01-1"
    $sha256sum = "E48AF208D1A46F47490011979538C1DBC427B36D47554A76C3FF83284E8F83AE"
} else {
    throw ('unknown CMAKE_CONFIGURATION: ' + "$env:CMAKE_CONFIGURATION")
}
$archive = "$filename.zip"

$outdir = $pwd.Path
$outdir = "$outdir\.gitlab"
$ProgressPreference = 'SilentlyContinue'
# This URL is only visible inside of Kitware's network.  See above filename table.
Invoke-WebRequest -Uri "https://cmake.org/files/dependencies/internal/$archive" -OutFile "$outdir\$archive"
$hash = Get-FileHash "$outdir\$archive" -Algorithm SHA256
if ($hash.Hash -ne $sha256sum) {
    exit 1
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory("$outdir\$archive", "$outdir")
Move-Item -Path "$outdir\$filename" -Destination "$outdir\pellesc"
Remove-Item "$outdir\$archive"

$scripts = "povars32.bat", "povars64.bat"
foreach ($script in $scripts) {
    $bat = Get-Content -Path "$outdir\pellesc\bin\$script.in" -Raw
    $bat = $bat -Replace "@PellesCDir@","$outdir\pellesc"
    $bat | Set-Content -Path "$outdir\pellesc\bin\$script"
}

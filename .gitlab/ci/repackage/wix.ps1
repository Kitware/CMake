# WiX Toolset 4+ is provided only via nuget packages.
# Download the package artifacts, extract the parts we need, and repackage them.

param (
  [Parameter(Mandatory=$true)]
  [string]$version
  )

$erroractionpreference = "stop"

$version_major = $version.Substring(0, $version.IndexOf('.'))

$release = "v" + $version
$pkg_wix = "wix.$version.nupkg"
$pkg_wixui = "WixToolset.UI.wixext.$version.nupkg"
$packages = $pkg_wix, $pkg_wixui

$wix_artifacts = "wix-artifacts.zip"

$ProgressPreference = 'SilentlyContinue'
Invoke-WebRequest -Uri "https://github.com/wixtoolset/wix/releases/download/$release/artifacts.zip" -OutFile "$wix_artifacts"

Add-Type -AssemblyName System.IO.Compression.FileSystem

$zip = [System.IO.Compression.ZipFile]::Open("$wix_artifacts", "read")
$zip.Entries | Where-Object FullName -in $packages | ForEach-Object {
  [System.IO.Compression.ZipFileExtensions]::ExtractToFile($_, "$($_.Name)", $true)
}
$zip.Dispose()
Remove-Item "$wix_artifacts"

$wix_dir = "wix-$version-win-any-1"
[System.IO.Compression.ZipFile]::ExtractToDirectory($pkg_wix, "wix-tmp")
Move-Item -Path "wix-tmp/tools/net6.0/any" -Destination "$wix_dir"
Remove-Item "wix-tmp" -Recurse -Force
Remove-Item "$pkg_wix"

$ext_dir = New-Item -Force -ItemType Directory -Path "$wix_dir/.wix/extensions/WixToolset.UI.wixext/$version/wixext$version_major"
$zip = [System.IO.Compression.ZipFile]::Open($pkg_wixui, "read")
$zip.Entries | Where-Object Name -eq "WixToolset.UI.wixext.dll" | ForEach-Object {
  [System.IO.Compression.ZipFileExtensions]::ExtractToFile($_, (Join-Path $ext_dir $_.Name), $true)
}
$zip.Dispose()
Remove-Item "$pkg_wixui"

@"
This was extracted from WiX Toolset nuget packages and repackaged.
Point both PATH and WIX_EXTENSIONS environment variables at this directory.

"@ | Add-Content -NoNewline "$wix_dir/README.txt"

$compressionLevel = [System.IO.Compression.CompressionLevel]::Optimal
$includeBaseDirectory = $true
[System.IO.Compression.ZipFile]::CreateFromDirectory("$wix_dir", "$wix_dir.zip", $compressionLevel, $includeBaseDirectory)
Remove-Item "$wix_dir" -Recurse -Force

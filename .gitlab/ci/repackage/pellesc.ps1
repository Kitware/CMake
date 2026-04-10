# Pelles C compilers are available only via installers.
# Run an installer and repackage the installation directory.

# From the Pelles C download page, download "setup.exe".
# Run this script passing "setup.exe" followed by the version number.

#Requires -RunAsAdministrator

param (
  [Parameter(Mandatory=$true)]
  [string]$installer,
  [Parameter(Mandatory=$true)]
  [string]$version,
  [string]$revision = "1",
  [string]$basedir = "c:\PellesC"
  )

$erroractionpreference = "stop"

Add-Type -AssemblyName System.IO.Compression.FileSystem

$installer_file = Get-Item $installer
$installer_name = $installer_file.Name
$package_name = "pellesc-$version-$revision"
$package_dir = New-Item -Force -ItemType Directory -Path "$basedir\$package_name"
if (-not $package_dir) {
  Write-Host "Failed to create package install dir."
  Exit 1
}

Write-Host "Installing to: $package_dir"

# The installer treats everything after /D= as the destination.
# Start-Process adds a trailing space, so use "cmd" instead.
& "$env:ComSpec" /c start /wait "$installer_file" /S /D=$package_dir
Write-Host ""
Remove-Item "$package_dir/uninst.exe" -Force

# Convert environment scripts to templates.
$bats = @(
  "bin\povars32.bat"
  "bin\povars64.bat"
  )
foreach ($p in $bats) {
  $bat = Get-Content -Path "$package_dir\$p" -Raw
  $bat = $bat -Replace "PellesCDir=.*","PellesCDir=@PellesCDir@"
  $bat | Set-Content -Path "$package_dir\$p.in"
  Remove-Item "$package_dir\$p" -Force
}

@"
This was repackaged from an installation by

  Pelles C $version "$installer_name"

using CMake's ".gitlab/ci/repackage/pellesc.ps1" script.

Copy or rename the environment script templates in the "bin" directory:

  povars32.bat.in => povars32.bat
  povars64.bat.in => povars64.bat

and replace the "@PellesCDir@" placeholders with

  C:\path\to\this\directory

Then use "povars32.bat" or "povars64.bat" to establish an environment.

"@ | Add-Content -NoNewline "$package_dir/README.txt"

Write-Host "Repackaging to: $package_name.zip"
$compressionLevel = [System.IO.Compression.CompressionLevel]::Optimal
$includeBaseDirectory = $true
[System.IO.Compression.ZipFile]::CreateFromDirectory("$package_dir", "$package_name.zip", $compressionLevel, $includeBaseDirectory)

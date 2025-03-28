# IAR Tooling for Windows is available only via installers.
# Run an installer and repackage the installation directory.

#Requires -RunAsAdministrator

param (
  [Parameter(Mandatory=$true)]
  [string]$installer,
  [string]$revision = "1",
  [string]$basedir = "c:\iar"
  )

$erroractionpreference = "stop"

Add-Type -AssemblyName System.IO.Compression.FileSystem

$installer_file = Get-Item $installer
$installer_name = $installer_file.Name
$package_name = $installer_file.Basename + "-" + $revision
$package_dir = "$basedir\$package_name"


Write-Host "Installing to: $package_dir"
Start-Process -Wait -FilePath "$installer_file" -ArgumentList "/hide_usd /autoinstall/$package_dir"
@"
This was repackaged from an installation by "$installer_name".
Obtain a network license as follows:

  set IAR_LMS_SETTINGS_DIR=%cd%\license
  %cd%\common\bin\lightlicensemanager setup --host %LicenseServerHostname%

"@ | Add-Content -NoNewline "$package_dir/README.txt"


Write-Host "Repackaging to: $package_name.zip"
$compressionLevel = [System.IO.Compression.CompressionLevel]::Optimal
$includeBaseDirectory = $true
[System.IO.Compression.ZipFile]::CreateFromDirectory("$package_dir", "$package_name.zip", $compressionLevel, $includeBaseDirectory)

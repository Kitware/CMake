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
$exclude = @(
  "arm/config/debugger"
  "arm/config/flashloader"
  "arm/doc"
  "arm/drivers"
  "arm/src"
  "avr/doc"
  "avr/src/cstat"
  "avr/src/lib"
  "avr/src/lib_tiny"
  "avr/src/linker"
  "riscv/config/debugger"
  "riscv/config/flashloader"
  "riscv/doc"
  "riscv/drivers"
  "riscv/src"
  "rh850/config/debugger"
  "rh850/doc"
  "rh850/src"
  "rl78/config/debugger"
  "rl78/config/renesas"
  "rl78/doc"
  "rl78/drivers"
  "rl78/src"
  "rx/config/debugger"
  "rx/config/flashloader"
  "rx/drivers"
  "rx/src"
  )

Write-Host "Installing to: $package_dir"
Start-Process -Wait -FilePath "$installer_file" -ArgumentList "/hide_usd /autoinstall/$package_dir"
foreach ($p in $exclude) {
    if (Test-Path "$package_dir/$p") {
        Remove-Item "$package_dir/$p" -Recurse -Force
    }
}

@"
This was repackaged from an installation by "$installer_name"
using CMake's ".gitlab/ci/repackage/iar.ps1" script.

Obtain a network license as follows:

  set IAR_LMS_SETTINGS_DIR=%cd%\license
  %cd%\common\bin\lightlicensemanager setup --host %LicenseServerHostname%

"@ | Add-Content -NoNewline "$package_dir/README.txt"


Write-Host "Repackaging to: $package_name.zip"
$compressionLevel = [System.IO.Compression.CompressionLevel]::Optimal
$includeBaseDirectory = $true
[System.IO.Compression.ZipFile]::CreateFromDirectory("$package_dir", "$package_name.zip", $compressionLevel, $includeBaseDirectory)

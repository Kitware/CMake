# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Run this script on a Windows host in a CMake single-config build tree.

param (
  [string]$signtool = 'signtool',
  [string]$cpack = 'bin\cpack',
  [switch]$trace
)

if ($trace -eq $true) {
  Set-PSDebug -Trace 1
}

$ErrorActionPreference = 'Stop'

# Sign binaries with SHA-1 for Windows 7 and below.
& $signtool sign -v -a -t http://timestamp.digicert.com -fd sha1 bin\*.exe

# Sign binaries with SHA-256 for Windows 8 and above.
& $signtool sign -v -a -tr http://timestamp.digicert.com -fd sha256 -td sha256 -as bin\*.exe

# Create packages.
& $cpack -G ZIP
& $cpack -G WIX

# Sign installer with SHA-256.
& $signtool sign -v -a -tr http://timestamp.digicert.com -fd sha256 -td sha256 -d "CMake Windows Installer" cmake-*-win*.msi

# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Run this script on a Windows host in a CMake single-config build tree.

param (
  [string]$signtool = 'signtool',
  [string]$cpack = 'bin\cpack',
  [string]$pass = $null
)

$ErrorActionPreference = 'Stop'

# Cleanup temporary file(s) on exit.
$null = Register-EngineEvent PowerShell.Exiting -Action {
  if ($certFile) {
    Remove-Item $certFile -Force
  }
}

# If the passphrase was not provided on the command-line,
# check for a GitLab CI variable in the environment.
if (-not $pass) {
  $pass = $env:SIGNTOOL_PASS

  # If the environment variable looks like a GitLab CI file-type variable,
  # replace it with the content of the file.
  if ($pass -and
      $pass.EndsWith("SIGNTOOL_PASS") -and
      (Test-Path -Path "$pass" -IsValid) -and
      (Test-Path -Path "$pass" -PathType Leaf)) {
    $pass = Get-Content -Path "$pass"
  }
}

# Collect signtool arguments to specify a certificate.
$cert = @()

# Select a signing certificate to pass to signtool.
if ($certX509 = Get-ChildItem -Recurse -Path "Cert:" -CodeSigningCert |
                  Where-Object { $_.PublicKey.Oid.FriendlyName -eq "RSA" } |
                  Select-Object -First 1) {
  # Identify the private key provider name and container name.
  if ($certRSA = [System.Security.Cryptography.X509Certificates.RSACertificateExtensions]::GetRSAPrivateKey($certX509)) {
    # $certRSA -is [System.Security.Cryptography.RSACng]
    # Cryptography Next Generation (CNG) implementation
    $csp = $certRSA.Key.Provider
    $kc = $certRSA.Key.KeyName
  } elseif ($certRSA = $certX509.PrivateKey) {
    # $certRSA -is [System.Security.Cryptography.RSACryptoServiceProvider]
    $csp = $certRSA.CspKeyContainerInfo.ProviderName
    $kc = $certRSA.CspKeyContainerInfo.KeyContainerName
  }

  # Pass the selected certificate to signtool.
  $certFile = New-TemporaryFile
  $certBase64 = [System.Convert]::ToBase64String($certX509.RawData, [System.Base64FormattingOptions]::InsertLineBreaks)
  $certPEM = "-----BEGIN CERTIFICATE-----", $certBase64, "-----END CERTIFICATE-----" -join "`n"
  $certPEM | Out-File -FilePath "$certFile" -Encoding Ascii
  $cert += "-f","$certFile"

  # Tell signtool how to find the certificate's private key.
  if ($csp) {
    $cert += "-csp","$csp"
  }
  if ($kc) {
    if ($pass) {
      # The provider offers a syntax to encode the token passphrase in the key container name.
      # https://web.archive.org/web/20250315200813/https://stackoverflow.com/questions/17927895/automate-extended-validation-ev-code-signing-with-safenet-etoken
      $cert += "-kc","[{{$pass}}]=$kc"
      $pass = $null
    } else {
      $cert += "-kc","$kc"
    }
  }
} else {
  $cert += @("-a")
}

# Sign binaries with SHA-1 for Windows 7 and below.
& $signtool sign -v $cert -t  http://timestamp.digicert.com -fd sha1 bin\*.exe
if (-not $?) { Exit $LastExitCode }

# Sign binaries with SHA-256 for Windows 8 and above.
& $signtool sign -v $cert -tr http://timestamp.digicert.com -fd sha256 -td sha256 -as bin\*.exe
if (-not $?) { Exit $LastExitCode }

# Create packages.
& $cpack -G "ZIP;WIX"
if (-not $?) { Exit $LastExitCode }

# Sign installer with SHA-256.
& $signtool sign -v $cert -tr http://timestamp.digicert.com -fd sha256 -td sha256 -d "CMake Windows Installer" cmake-*-win*.msi
if (-not $?) { Exit $LastExitCode }

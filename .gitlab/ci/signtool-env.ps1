if ("$env:PROCESSOR_ARCHITECTURE" -eq "AMD64") {
    $arch = "x64"
} elseif ("$env:PROCESSOR_ARCHITECTURE" -eq "ARM64") {
    $arch = "arm64"
} else {
    throw ('unknown PROCESSOR_ARCHITECTURE: ' + "$env:PROCESSOR_ARCHITECTURE")
}

$regKey = 'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\Windows\v10.0'
$signtoolPath = $null
if ($sdkDir = Get-ItemPropertyValue -Path $regKey -Name "InstallationFolder") {
  if ($sdkBin = Get-ChildItem -Path "$sdkDir/bin" -Recurse -Name "$arch" |
                  Where-Object { Test-Path -Path "$sdkDir/bin/$_/signtool.exe" -PathType Leaf } |
                  Select-Object -Last 1) {
    $signtoolPath = "$sdkDir/bin/$sdkBin"
  }
}
if ($signtoolPath) {
  Set-Item -Force -Path "env:PATH" -Value "$env:PATH;$signtoolPath"
} else {
  throw ('No signtool.exe found in Windows SDK')
}

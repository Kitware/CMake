$pwsh = [System.Diagnostics.Process]::GetCurrentProcess().MainModule.FileName
if (Test-Path -Path ".gitlab/ci/env_$env:CMAKE_CONFIGURATION.ps1" -PathType Leaf) {
  . ".gitlab/ci/env_$env:CMAKE_CONFIGURATION.ps1"
}

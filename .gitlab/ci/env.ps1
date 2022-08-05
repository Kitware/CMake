if (Test-Path -Path ".gitlab/ci/env_$env:CMAKE_CONFIGURATION.ps1" -PathType Leaf) {
  Invoke-Expression ".gitlab/ci/env_$env:CMAKE_CONFIGURATION.ps1"
}

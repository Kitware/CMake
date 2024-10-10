if ("$env:CMAKE_CI_NIGHTLY" -eq "true") {
  . ".gitlab/ci/innosetup-env.ps1"
  . ".gitlab/ci/ispc-env.ps1"
  . ".gitlab/ci/nuget-env.ps1"
  . ".gitlab/ci/swift-env.ps1"
}

& "$pwsh" -File .gitlab/ci/wix3.ps1
& "$pwsh" -File .gitlab/ci/wix4.ps1

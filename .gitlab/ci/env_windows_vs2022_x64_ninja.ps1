if ("$env:CMAKE_CI_NIGHTLY" -eq "true") {
  Invoke-Expression ".gitlab/ci/ispc-env.ps1"
}

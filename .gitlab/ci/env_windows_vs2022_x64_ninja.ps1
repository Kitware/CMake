if ("$env:CMAKE_CI_NIGHTLY" -eq "true") {
  . ".gitlab/ci/innosetup-env.ps1"
  . ".gitlab/ci/ispc-env.ps1"
}

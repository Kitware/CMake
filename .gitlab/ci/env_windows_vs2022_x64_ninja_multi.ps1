if ("$env:CMAKE_CI_NIGHTLY" -eq "true") {
  . ".gitlab/ci/ispc-env.ps1"
}

. .gitlab/ci/ninja-env.ps1

Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1

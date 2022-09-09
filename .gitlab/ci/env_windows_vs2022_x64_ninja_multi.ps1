if ("$env:CMAKE_CI_NIGHTLY" -eq "true") {
  . ".gitlab/ci/ispc-env.ps1"
}

$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/ninja.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab;$env:PATH"
ninja --version

Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1

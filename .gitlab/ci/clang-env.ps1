Invoke-Expression -Command .gitlab/ci/clang.ps1
Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1

$pwdpath = $pwd.Path
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\llvm\bin;$env:PATH"

if ("$env:CMAKE_CI_BUILD_NAME" -match "(^|_)gnu(_|$)") {
  $env:CC  = "clang"
  $env:CXX = "clang++"
  clang --version
} else {
  $env:CC  = "clang-cl"
  $env:CXX = "clang-cl"
  clang-cl --version
}

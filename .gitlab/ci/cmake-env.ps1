$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/cmake.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\cmake\bin;$env:PATH"
cmake --version
$cmake = "cmake"

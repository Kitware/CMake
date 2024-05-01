Invoke-Expression -Command .gitlab/ci/jom.ps1
$pwdpath = $pwd.Path
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\jom;$env:PATH"
Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1

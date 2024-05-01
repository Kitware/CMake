Invoke-Expression -Command .gitlab/ci/orangec.ps1
$pwdpath = $pwd.Path
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\orangec\bin;$env:PATH"
Set-Item -Force -Path "env:ORANGEC" -Value "$pwdpath\.gitlab\orangec"

$env:CC  = "occ"
$env:CXX = "occ"
occ --version

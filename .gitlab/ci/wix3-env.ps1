& "$pwsh" -File .gitlab/ci/wix3.ps1

$pwdpath = $pwd.Path
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\wix3;$env:PATH"

light -help | Select -First 1

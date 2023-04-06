Invoke-Expression -Command .gitlab/ci/borland.ps1
$pwdpath = $pwd.Path
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\bcc\bin;$env:PATH"

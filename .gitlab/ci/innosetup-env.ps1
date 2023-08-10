$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/innosetup.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\innosetup;$env:PATH"
ISCC 2>$null | Select -First 1

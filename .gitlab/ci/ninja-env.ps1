$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/ninja.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab;$env:PATH"
Write-Host "ninja version: $(ninja --version)"

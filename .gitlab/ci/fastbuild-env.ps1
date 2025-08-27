$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/fastbuild.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\fastbuild;$env:PATH"
Write-Host "fbuild version: $(fbuild -version)"

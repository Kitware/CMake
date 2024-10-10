& "$pwsh" -File .gitlab/ci/wix4.ps1

$pwdpath = $pwd.Path
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\wix4;$env:PATH"
Set-Item -Force -Path "env:WIX_EXTENSIONS" -Value "$pwdpath\.gitlab\wix4"
Write-Host "wix version: $(wix --version)"
Write-Host "wix extensions: $(wix extension list -g)"

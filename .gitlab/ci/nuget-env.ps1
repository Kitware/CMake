$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/nuget.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\nuget;$env:PATH"
nuget | Select -First 1

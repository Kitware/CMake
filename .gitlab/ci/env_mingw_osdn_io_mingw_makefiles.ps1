$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/mingw.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\mingw\bin;$env:PATH"

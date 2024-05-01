$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/python.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\python3;$env:PATH"
python --version

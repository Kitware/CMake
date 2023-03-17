$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/mingw.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\mingw\msys\1.0\bin;$pwdpath\.gitlab\mingw\bin;$env:PATH"
$env:MSYSTEM = 'MINGW32'
$env:MAKE_MODE = 'unix'

Invoke-Expression -Command .gitlab/ci/openwatcom.ps1
$pwdpath = $pwd.Path
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\watcom\binnt;$pwdpath\.gitlab\watcom\binw;$env:PATH"
Set-Item -Force -Path "env:INCLUDE" -Value "$pwdpath\.gitlab\watcom\h;$pwdpath\.gitlab\watcom\h\nt"
Set-Item -Force -Path "env:EDPATH" -Value "$pwdpath\.gitlab\watcom\eddat"
Set-Item -Force -Path "env:WATCOM" -Value "$pwdpath\.gitlab\watcom"
Set-Item -Force -Path "env:WLINKTMP" -Value "."

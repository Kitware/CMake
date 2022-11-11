$pwdpath = $pwd.Path
cmake -P .gitlab/ci/download_qt.cmake
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\qt\bin;$env:PATH"
qmake -v

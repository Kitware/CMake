$pwdpath = $pwd.Path
cmake -P .gitlab/ci/download_python3.cmake
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\python3;$env:PATH"
python --version

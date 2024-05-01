$erroractionpreference = "stop"

Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1
Set-Location -Path ".gitlab"
git clone https://github.com/ninja-build/ninja.git ninja-src
cmake -S ninja-src -B ninja-src/build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build ninja-src/build --target ninja
Move-Item -Path "ninja-src\build\ninja.exe" -Destination . -Force
Remove-Item "ninja-src" -Recurse -Force

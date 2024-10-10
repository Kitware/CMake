if (Test-Path -Path "build/ci_package_info.cmake" -PathType Leaf) {
    cmake -P .gitlab/ci/package_windows_build.cmake
} else {
    cd build
    cpack -G ZIP
    cpack -G WIX
}

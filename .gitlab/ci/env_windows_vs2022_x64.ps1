if ("$env:CMAKE_CI_NIGHTLY" -eq "true") {
  $cmake = "build\install\bin\cmake"
  . ".gitlab/ci/qt-env.ps1"
}

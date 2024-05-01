$pwsh = [System.Diagnostics.Process]::GetCurrentProcess().MainModule.FileName
if (Test-Path -Path ".gitlab/ci/post_build_$env:CMAKE_CONFIGURATION.ps1" -PathType Leaf) {
  . ".gitlab/ci/post_build_$env:CMAKE_CONFIGURATION.ps1"
}

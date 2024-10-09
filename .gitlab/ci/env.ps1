$pwsh = [System.Diagnostics.Process]::GetCurrentProcess().MainModule.FileName

# Place temporary files inside job directory.
$tmp = New-Item -Force -ItemType Directory -Path "$pwd\.gitlab\tmp"
$tmp = (New-Object -ComObject Scripting.FileSystemObject).GetFolder("$tmp").ShortPath
Set-Item -Force -Path "env:TEMP" -Value "$tmp"
Set-Item -Force -Path "env:TMP"  -Value "$tmp"
$tmp = $null

if (Test-Path -Path ".gitlab/ci/env_$env:CMAKE_CONFIGURATION.ps1" -PathType Leaf) {
  . ".gitlab/ci/env_$env:CMAKE_CONFIGURATION.ps1"
}

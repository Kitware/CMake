if ($cmake -eq $null) {
    throw ('$cmake powershell variable not set ')
}
if ("$env:PROCESSOR_ARCHITECTURE" -eq "AMD64") {
    $pwdpath = $pwd.Path
    & $cmake -P .gitlab/ci/download_qt.cmake
    Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\qt\bin;$env:PATH"
    qmake -v
} elseif ("$env:PROCESSOR_ARCHITECTURE" -eq "ARM64") {
    # Qt host tools are not yet available natively on windows-arm64.
} else {
    throw ('unknown PROCESSOR_ARCHITECTURE: ' + "$env:PROCESSOR_ARCHITECTURE")
}

$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/iar.ps1"
Set-Item -Force -Path "env:IAR_LMS_SETTINGS_DIR" -Value "$pwdpath\.gitlab\iar\license"
$exes = Get-Item -Path "$pwdpath\.gitlab\iar\*\*\bin\icc*.exe"
$exes | ForEach-Object { Write-Host $_.FullName }

if ($env:CMAKE_CI_IAR_LICENSE_SERVER) {
    $llms = Get-Item -Path "$pwdpath\.gitlab\iar\*\common\bin\lightlicensemanager.exe"
    foreach ($llm in $llms) {
        &$llm.FullName setup --host "$env:CMAKE_CI_IAR_LICENSE_SERVER"
    }
    foreach ($exe in $exes) {
        &$exe.FullName --version
    }
}

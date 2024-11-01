$erroractionpreference = "stop"

$all_env = cmd /c "`"$env:VCVARSALL`" $env:VCVARSPLATFORM -vcvars_ver=$env:VCVARSVERSION >NUL & powershell -Command `"Get-ChildItem env: | Select-Object -Property Key,Value | ConvertTo-Json`"" | ConvertFrom-Json

foreach ($envvar in $all_env) {
    [Environment]::SetEnvironmentVariable($envvar.Key, $envvar.Value)
}

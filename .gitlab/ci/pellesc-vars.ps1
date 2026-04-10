$erroractionpreference = "stop"

$all_env = cmd /c "`".gitlab\pellesc\bin\povars64.bat`" >NUL & powershell -Command `"Get-ChildItem env: | Select-Object -Property Key,Value | ConvertTo-Json`"" | ConvertFrom-Json

foreach ($envvar in $all_env) {
    [Environment]::SetEnvironmentVariable($envvar.Key, $envvar.Value)
}

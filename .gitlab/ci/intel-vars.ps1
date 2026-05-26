$erroractionpreference = "stop"

$all_env = cmd /c "`".gitlab\intel\setvars.bat`" >NUL & powershell -Command `"Get-ChildItem env: | Select-Object -Property Key,Value | ConvertTo-Json`"" | ConvertFrom-Json

foreach ($envvar in $all_env) {
    [Environment]::SetEnvironmentVariable($envvar.Key, $envvar.Value)
}

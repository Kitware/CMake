$erroractionpreference = "stop"

cmd /c "`".gitlab\intel\setvars.bat`" & set" |
foreach {
    if ($_ -match "=") {
        $v = $_.split("=")
        [Environment]::SetEnvironmentVariable($v[0], $v[1])
    }
}

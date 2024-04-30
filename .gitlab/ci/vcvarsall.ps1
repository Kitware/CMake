$erroractionpreference = "stop"

cmd /c "`"$env:VCVARSALL`" $env:VCVARSPLATFORM -vcvars_ver=$env:VCVARSVERSION & set" |
foreach {
    if ($_ -match "=") {
        $v = $_.split("=")
        [Environment]::SetEnvironmentVariable($v[0], $v[1])
    }
}

$erroractionpreference = "stop"

cmd /c "`"$env:VCVARSALL`" $VCVARSPLATFORM -vcvars_ver=$VCVARSVERSION & set" |
foreach {
    if ($_ -match "=") {
        $v = $_.split("=")
        [Environment]::SetEnvironmentVariable($v[0], $v[1])
    }
}

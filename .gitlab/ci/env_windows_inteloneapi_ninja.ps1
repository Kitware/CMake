. .gitlab/ci/ninja-env.ps1
. .gitlab/ci/intel-env.ps1

$env:CC  = "icx"
$env:CXX = "icx"
$env:FC  = "ifx"

cmd /c "icx 2>&1" | Select -First 1
cmd /c "ifx 2>&1" | Select -First 1

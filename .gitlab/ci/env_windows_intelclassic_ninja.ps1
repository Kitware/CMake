. .gitlab/ci/ninja-env.ps1
. .gitlab/ci/intel-env.ps1

$env:CC  = "icl"
$env:CXX = "icl"
$env:FC  = "ifort"

cmd /c "icl 2>&1" | Select -First 1
cmd /c "ifort 2>&1" | Select -First 1

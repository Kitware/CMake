$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/intel.ps1"
Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1
Invoke-Expression -Command .gitlab/ci/intel-vars.ps1

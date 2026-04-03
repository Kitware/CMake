& "$pwsh" -File ".gitlab/ci/pellesc.ps1"
Invoke-Expression -Command .gitlab/ci/pellesc-vars.ps1

$env:CC  = "pocc"
$env:CXX = "pocc-does-not-support-c++"
pocc | Select -First 1

cd build
. ../Utilities/Release/win/sign-package.ps1 -cpack cpack
if (-not $?) { Exit $LastExitCode }

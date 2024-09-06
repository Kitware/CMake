if ("$env:CMAKE_CI_NIGHTLY" -eq "true") {
  if ("$env:CI_JOB_STAGE" -ne "build") {
    # As a special case, we do not actually fetch IAR tooling
    # in the build job.  It is not used until the test job.
    . ".gitlab/ci/iar-env.ps1"
  }
  . ".gitlab/ci/innosetup-env.ps1"
  . ".gitlab/ci/ispc-env.ps1"
  . ".gitlab/ci/nuget-env.ps1"
  . ".gitlab/ci/swift-env.ps1"
}

& "$pwsh" -File .gitlab/ci/wix3.ps1
& "$pwsh" -File .gitlab/ci/wix4.ps1

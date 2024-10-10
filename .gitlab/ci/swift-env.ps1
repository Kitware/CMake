$pwdpath = $pwd.Path
& "$pwsh" -File ".gitlab/ci/swift.ps1"
Set-Item -Force -Path "env:DEVELOPER_DIR" -Value "$pwdpath\.gitlab\swift"
Set-Item -Force -Path "env:SDKROOT" -Value "$pwdpath\.gitlab\swift\Platforms\Windows.platform\Developer\SDKs\Windows.sdk"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\swift\Toolchains\unknown-Asserts-development.xctoolchain\usr\bin;$env:PATH"
swiftc --version

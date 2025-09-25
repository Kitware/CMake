# Intel Compilers for Windows are available only via installers.
# Run an installer and repackage the installation directory.

# From the Intel oneAPI download page, download offline
# installers for the Base and HPC products, as documented
# by their "Install through a Command Line" sections.

#Requires -RunAsAdministrator

param (
  [Parameter(Mandatory=$true)]
  [string]$installer,
  [string]$revision = "1",
  [string]$basedir = "c:\intel"
  )

$erroractionpreference = "stop"

Add-Type -AssemblyName System.IO.Compression.FileSystem

$base_installer_file = Get-Item -Path $installer
$base_installer_name = $base_installer_file.Name
if ($base_installer_name -match '^intel-oneapi-base-toolkit-(?<version>(?<version2>[0-9][0-9][0-9][0-9]\.[0-9])\.[0-9])\.') {
  $version2 = $Matches.version2
  $version = $Matches.version
} else {
  Write-Host "Base installer file does not match expected pattern."
  Exit 1
}

$hpc_installer_file = Get-Item -Path (Join-Path $base_installer_file.Directory "intel-oneapi-hpc-toolkit-$version.*")
if (-not $hpc_installer_file) {
  Write-Host "HPC installer file not found next to base installer."
  Exit 1
}
$hpc_installer_name = $hpc_installer_file.Name
Write-Host "Version: '$version'"
Write-Host "Base Installer: '$base_installer_file'"
Write-Host "HPC Installer: '$hpc_installer_file'"

$package_name = "intel-oneapi-$version-windows-$revision"
$package_dir = New-Item -Force -ItemType Directory -Path "$basedir\$package_name"
if (-not $package_dir) {
  Write-Host "Failed to create package install dir."
  Exit 1
}
$compiler_exclude = @(
  ".toolkit_linking_tool"
  "bin\*.o"
  "bin\*.rtl"
  "bin\*.spv"
  "bin\1033"
  "bin\OpenCL.dll"
  "bin\aocl-ioc64.exe"
  "bin\cl.cfg"
  "bin\common_clang64.dll"
  "bin\compiler\append-file.exe"
  "bin\compiler\clang++.exe"
  "bin\compiler\clang-cl.exe"
  "bin\compiler\clang-cpp.exe"
  "bin\compiler\clang-format.exe"
  "bin\compiler\clang-include-fixer.exe"
  "bin\compiler\clang-linker-wrapper.exe"
  "bin\compiler\clang-offload-*.exe"
  "bin\compiler\clang-tidy.exe"
  "bin\compiler\clangd.exe"
  "bin\compiler\file-table-tform.exe"
  "bin\compiler\ld.lld.exe"
  "bin\compiler\lld-link.exe"
  "bin\compiler\llvm-cov.exe"
  "bin\compiler\llvm-lib.exe"
  "bin\compiler\llvm-profdata.exe"
  "bin\compiler\llvm-profgen.exe"
  "bin\compiler\llvm-spirv.exe"
  "bin\compiler\llvm-symbolizer.exe"
  "bin\compiler\modularize.exe"
  "bin\compiler\spirv-to-ir-wrapper.exe"
  "bin\compiler\sycl-post-link.exe"
  "bin\compiler\yaml2obj.exe"
  "bin\deftofd.exe"
  "bin\dpcpp-cl.exe"
  "bin\dpcpp.exe"
  "bin\icpx.exe"
  "bin\icx-cc.exe"
  "bin\icx-cl.exe"
  "bin\intelocl64.*"
  "bin\ioc64.exe"
  "bin\libhwloc-15.dll"
  "bin\libintelocl.so-gdb.py"
  "bin\libiomp5md.pdb"
  "bin\libocl_*.dll"
  "bin\opencl-aot.exe"
  "bin\pstloffload*.dll"
  "bin\run-clang-tidy"
  "bin\svml_dispmd.dll"
  "bin\sycl*"
  "bin\tcm*"
  "bin\ur_*.dll"
  "share"
  )

Write-Host "Installing to: $package_dir"
$install_args = "-s -r yes -a --silent --eula accept"
$install_args = $install_args + " --instance repackage"
$install_args = $install_args + " -p=NEED_VS2019_INTEGRATION=0"
$install_args = $install_args + " -p=NEED_VS2022_INTEGRATION=0"
Start-Process -Wait -FilePath "$base_installer_file" `
  -ArgumentList "$install_args --install-dir $package_dir --components intel.oneapi.win.cpp-dpcpp-common"
Write-Host ""
Start-Process -Wait -FilePath "$hpc_installer_file" `
  -ArgumentList "$install_args --components intel.oneapi.win.ifort-compiler"
Write-Host ""
Get-Item -Path "$package_dir\*" -Exclude compiler,setvars.bat,setvars-vcvarsall.bat | ForEach-Object {
  Remove-Item "$_" -Recurse -Force
}
Remove-Item "$package_dir/compiler/latest" -Recurse -Force
foreach ($p in $compiler_exclude) {
  Remove-Item "$package_dir/compiler/$version2/$p" -Recurse -Force
}

@"
This was repackaged from an installation by:

* $base_installer_name
* $hpc_installer_name

using CMake's ".gitlab/ci/repackage/intel.ps1" script.

Duplicate files were removed from this distribution.
Restore them using hard links:

* compiler/$version2/bin/compiler/clang++.exe    -> clang.exe
* compiler/$version2/bin/compiler/clang-cl.exe   -> clang.exe
* compiler/$version2/bin/compiler/clang-cpp.exe  -> clang.exe
* compiler/$version2/bin/compiler/ld.lld.exe     -> lld.exe
* compiler/$version2/bin/compiler/lld-link.exe   -> lld.exe
* compiler/$version2/bin/compiler/llvm-lib.exe   -> llvm-ar.exe
* compiler/$version2/bin/icpx.exe                -> icx.exe
* compiler/$version2/bin/icx-cc.exe              -> icx.exe
* compiler/$version2/bin/icx-cl.exe              -> icx.exe

Also add a directory junction:

* compiler/latest -> $version2

Then use "setvars.bat" to establish an environment.

"@ | Add-Content -NoNewline "$package_dir/README.txt"

Write-Host "Repackaging to: $package_name.zip"
$compressionLevel = [System.IO.Compression.CompressionLevel]::Optimal
$includeBaseDirectory = $true
[System.IO.Compression.ZipFile]::CreateFromDirectory("$package_dir", "$package_name.zip", $compressionLevel, $includeBaseDirectory)

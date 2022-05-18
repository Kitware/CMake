# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Run this script on a Windows host to generate Qt binaries.
# Set the PATH environment variable to contain the locations of cmake and git.

param (
  [string]$cmake = 'cmake',
  [string]$git = 'git',
  [switch]$trace
)

if ($trace -eq $true) {
  Set-PSDebug -Trace 1
}

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

if ($env:VSCMD_ARG_TGT_ARCH -eq "x64") {
  $arch = "x86_64";
} elseif ($env:VSCMD_ARG_TGT_ARCH -eq "x86") {
  $arch = "i386";
} else {
  Write-Host "VSCMD_ARG_TGT_ARCH env var not recognized.  Run this from a Visual Studio Command Prompt."
  exit 1
}

if ($env:VCToolsVersion -match '^(?<version>[0-9][0-9]\.[0-9])') {
  $toolset = "msvc_v" + $Matches.version -replace '\.', ''
} else {
  Write-Host "VCToolsVersion env var not set.  Run this from a Visual Studio Command Prompt."
}

$srcname = "qt-everywhere-src-5.12.1"
$pkgname = "qt-5.12.1-win-$arch-$toolset-1"
$topdir = $pwd.Path
$srcdir = Join-Path $topdir $srcname
$blddir = Join-Path $topdir "$pkgname-build"
$prefix = Join-Path $topdir $pkgname

# JOM
if ( -not (Test-Path -Path "jom")) {
  Invoke-WebRequest -Uri "http://download.qt-project.org/official_releases/jom/unstable-jom.zip" -OutFile jom.zip
  if ($(Get-FileHash "jom.zip").Hash -ne '128fdd846fe24f8594eed37d1d8929a0ea78df563537c0c1b1861a635013fff8') {
      exit 1
  }
  Expand-Archive -Path jom.zip -DestinationPath jom
  Remove-Item jom.zip
}
$jom = "$topdir\jom\jom.exe"

# Qt Source
if ( -not (Test-Path -Path $srcdir)) {
  Invoke-WebRequest -Uri "https://download.qt.io/official_releases/qt/5.12/5.12.1/single/qt-everywhere-src-5.12.1.tar.xz" -OutFile qt.tar.xz
  if ($(Get-FileHash "qt.tar.xz").Hash -ne 'caffbd625c7bc10ff8c5c7a27dbc7d84fa4de146975c0e1ffe904b514ccd6da4') {
      exit 1
  }
  & $cmake -E tar xvf qt.tar.xz
  Remove-Item qt.tar.xz
}

# Build Qt
if ( -not (Test-Path -Path $blddir)) {
  New-Item -ItemType Directory -Path $blddir
  Set-Location -Path "$blddir"
  & ..\$srcname\configure.bat `
    -prefix $prefix `
    -static `
    -static-runtime `
    -release `
    -opensource -confirm-license `
    -platform win32-msvc `
    -mp `
    -gui `
    -widgets `
    -qt-pcre `
    -qt-zlib `
    -qt-libpng `
    -qt-libjpeg `
    -no-gif `
    -no-icu `
    -no-pch `
    -no-angle `
    -no-opengl `
    -no-dbus `
    -no-harfbuzz `
    -no-accessibility `
    -skip declarative `
    -skip multimedia `
    -skip qtcanvas3d `
    -skip qtconnectivity `
    -skip qtdeclarative `
    -skip qtlocation `
    -skip qtmultimedia `
    -skip qtsensors `
    -skip qtserialport `
    -skip qtsvg `
    -skip qtwayland `
    -skip qtwebchannel `
    -skip qtwebengine `
    -skip qtwebsockets `
    -skip qtxmlpatterns `
    -nomake examples -nomake tests
  & $jom -J $env:NUMBER_OF_PROCESSORS
}

# Install Qt
if ( -not (Test-Path -Path $prefix)) {
  & $jom install
  # Patch the installation.
  Set-Location -Path $prefix
  & $git apply -v (Join-Path $PSScriptRoot qt-5.12.1-win-x86-msvc-install.patch)
}

# Package Qt
Set-Location -Path $topdir
& $cmake -E tar cf "$pkgname.zip" "--format=zip" "$pkgname"

# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Run this script on a Windows host to generate Qt binaries.
# Set the PATH environment variable to contain the locations of cmake and git.

param (
  [string]$cmake = 'cmake',
  [string]$git = 'git',
  [string]$ninja = 'ninja',
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
} elseif ($env:VSCMD_ARG_TGT_ARCH -eq "arm64") {
  $arch = "arm64";
} else {
  Write-Host "VSCMD_ARG_TGT_ARCH env var not recognized.  Run this from a Visual Studio Command Prompt."
  exit 1
}

if ($env:VCToolsVersion -match '^(?<version>[0-9][0-9]\.[0-9])') {
  $toolset = "msvc_v" + $Matches.version -replace '\.', ''
} else {
  Write-Host "VCToolsVersion env var not set.  Run this from a Visual Studio Command Prompt."
}

$srcname = "qt-everywhere-src-6.3.0"
$pkgname = "qt-6.3.0-win-$arch-$toolset-1"
$pkgname_host = "qt-6.3.0-win-x86_64-$toolset-1"
$topdir = $pwd.Path
$srcdir = Join-Path $topdir $srcname
$blddir = Join-Path $topdir "$pkgname-build"
$prefix = Join-Path $topdir $pkgname
$prefix_host = Join-Path $topdir "$pkgname_host"

# Qt Source
if ( -not (Test-Path -Path $srcdir)) {
  Invoke-WebRequest -Uri "https://download.qt.io/official_releases/qt/6.3/6.3.0/single/qt-everywhere-src-6.3.0.tar.xz" -OutFile qt.tar.xz
  if ($(Get-FileHash "qt.tar.xz").Hash -ne 'cd2789cade3e865690f3c18df58ffbff8af74cc5f01faae50634c12eb52dd85b') {
      exit 1
  }
  & $cmake -E tar xvf qt.tar.xz
  Remove-Item qt.tar.xz
}

# Build Qt
if ( -not (Test-Path -Path $blddir)) {
  New-Item -ItemType Directory -Path $blddir
  Set-Location -Path "$blddir"
  if ($arch -eq "arm64") {
    $qt_platform = "win32-arm64-msvc"
    $qt_host_path = "-qt-host-path", "$prefix_host"
  } else {
    $qt_platform = "win32-msvc"
    $qt_host_path = $null
  }
  & ..\$srcname\configure.bat `
    -prefix $prefix `
    -static `
    -static-runtime `
    -release `
    -opensource -confirm-license `
    -platform $qt_platform `
    $qt_host_path `
    -gui `
    -widgets `
    -qt-doubleconversion `
    -qt-freetype `
    -qt-harfbuzz `
    -qt-pcre `
    -qt-zlib `
    -qt-libpng `
    -qt-libjpeg `
    -no-gif `
    -no-tiff `
    -no-webp `
    -no-icu `
    -no-pch `
    -no-opengl `
    -no-dbus `
    -no-accessibility `
    -no-feature-androiddeployqt `
    -no-feature-assistant `
    -no-feature-designer `
    -no-feature-linguist `
    -no-feature-pixeltool `
    -no-feature-printsupport `
    -no-feature-qtattributionsscanner `
    -no-feature-qtdiag `
    -no-feature-qtplugininfo `
    -no-feature-sql `
    -no-feature-windeployqt `
    -skip qtconnectivity `
    -skip qtdeclarative `
    -skip qtdoc `
    -skip qtlottie `
    -skip qtmqtt `
    -skip qtmultimedia `
    -skip qtopcua `
    -skip qtquick3d `
    -skip qtquicktimeline `
    -skip qtscxml `
    -skip qtsensors `
    -skip qtserialport `
    -skip qtsvg `
    -skip qtvirtualkeyboard `
    -skip qtwayland `
    -skip qtwebchannel `
    -skip qtwebengine `
    -skip qtwebsockets `
    -skip qtwebview `
    -nomake examples `
    -nomake tests
  & $ninja
}

# Install Qt
if ( -not (Test-Path -Path $prefix)) {
  & $ninja install
}

# Package Qt
Set-Location -Path $topdir
& $cmake -E tar cf "$pkgname.zip" "--format=zip" "$pkgname"

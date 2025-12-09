# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Run this script in a Visual Studio Command Prompt to generate Qt binaries.

cmake_minimum_required(VERSION 3.23)

if ("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x64")
  set(arch "x86_64")
elseif ("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x86")
  set(arch "i386")
elseif ("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "arm64")
  set(arch "arm64")
else()
  message("VSCMD_ARG_TGT_ARCH env var not recognized.  Run this from a Visual Studio Command Prompt.")
  return()
endif()

if ("$ENV{VCToolsVersion}" MATCHES [[^([0-9][0-9])\.([0-9])]])
  set(toolset "msvc_v${CMAKE_MATCH_1}${CMAKE_MATCH_2}")
else()
  message( "VCToolsVersion='$ENV{VCToolsVersion}' env var not recognized.  Run this from a Visual Studio Command Prompt.")
  return()
endif()

set(srcname "qtbase-everywhere-src-6.10.1")
set(pkgname "qt-6.10.1-win-${arch}-${toolset}-1")
set(pkgname_host "qt-6.10.1-win-x86_64-${toolset}-1")
set(topdir "${CMAKE_CURRENT_BINARY_DIR}")
set(srcdir "${topdir}/${srcname}")
set(blddir "${topdir}/${pkgname}-b")
set(prefix "${topdir}/${pkgname}")
set(prefix_host "${topdir}/${pkgname_host}")

# Qt Source
if (NOT EXISTS "${srcdir}")
  file(DOWNLOAD "https://download.qt.io/official_releases/qt/6.10/6.10.1/submodules/qtbase-everywhere-src-6.10.1.tar.xz" qt.tar.xz
       EXPECTED_HASH SHA256=5a6226f7e23db51fdc3223121eba53f3f5447cf0cc4d6cb82a3a2df7a65d265d)
  file(ARCHIVE_EXTRACT INPUT qt.tar.xz)
  file(REMOVE qt.tar.xz)
endif()

# Build Qt
if (NOT EXISTS "${blddir}")
  file(MAKE_DIRECTORY "${blddir}")
  if ("${arch}" STREQUAL "arm64")
    set(qt_platform "win32-arm64-msvc")
    set(qt_host_path -qt-host-path "${prefix_host}")
  else()
    set(qt_platform "win32-msvc")
    unset(qt_host_path)
  endif()

  execute_process(
    RESULT_VARIABLE result
    WORKING_DIRECTORY "${blddir}"
    COMMAND
      ${srcdir}/configure.bat
      -prefix ${prefix}
      -static
      -static-runtime
      -release
      -opensource -confirm-license
      -platform ${qt_platform}
      ${qt_host_path}
      -gui
      -widgets
      -qt-doubleconversion
      -qt-freetype
      -qt-harfbuzz
      -qt-pcre
      -qt-zlib
      -qt-libpng
      -qt-libjpeg
      -no-gif
      -no-icu
      -no-pch
      -no-opengl
      -no-dbus
      -no-accessibility
      -no-feature-androiddeployqt
      -no-feature-printsupport
      -no-feature-sql
      -nomake examples
      -nomake tests
  )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "configure.bat failed: ${result}")
  endif()

  execute_process(
    RESULT_VARIABLE result
    WORKING_DIRECTORY "${blddir}"
    COMMAND ninja
  )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "ninja failed: ${result}")
  endif()
endif()

# Install Qt
if (NOT EXISTS "${prefix}")
  execute_process(
    RESULT_VARIABLE result
    WORKING_DIRECTORY "${blddir}"
    COMMAND ninja install
  )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "ninja install failed: ${result}")
  endif()
endif()

# Package Qt
file(ARCHIVE_CREATE OUTPUT "${pkgname}.zip" PATHS "${pkgname}" FORMAT "zip")

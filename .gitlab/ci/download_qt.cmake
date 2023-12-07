cmake_minimum_required(VERSION 3.12)

# Input variables.
set(qt_version_major "5")
set(qt_version_minor "15")
set(qt_version_patch "1")

# Combined version variables.
set(qt_version "${qt_version_major}.${qt_version_minor}.${qt_version_patch}")
set(qt_version_nodot "${qt_version_major}${qt_version_minor}${qt_version_patch}")

# Files needed to download.
set(qt_files)
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows.*package")
  set(qt_url_root "https://cmake.org/files/dependencies")
  set(qt_url_path "")
  if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows_x86_64_package")
    list(APPEND qt_files "qt-5.15.10-win-x86_64-msvc_v142-1.zip")
    set(qt_subdir "qt-5.15.10-win-x86_64-msvc_v142-1")
  elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows_i386_package")
    list(APPEND qt_files "qt-5.15.10-win-i386-msvc_v142-1.zip")
    set(qt_subdir "qt-5.15.10-win-i386-msvc_v142-1")
  elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows_arm64_package")
    list(APPEND qt_files "qt-6.3.0-win-arm64-msvc_v143-1.zip")
    set(qt_subdir "qt-6.3.0-win-arm64-msvc_v143-1")
    list(APPEND qt_files "qt-6.3.0-win-x86_64-msvc_v143-1.zip")
    set(qt_host_subdir "qt-6.3.0-win-x86_64-msvc_v143-1")
  else ()
    message(FATAL_ERROR "Unknown arch to use for Qt")
  endif()
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows")
  # Determine the ABI to fetch for Qt.
  if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2015")
    set(qt_platform "windows_x86")
    set(msvc_year "2015")
    set(qt_abi "win64_msvc${msvc_year}_64")
  elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2017" OR
          "$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019" OR
          "$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
    set(qt_platform "windows_x86")
    set(msvc_year "2019")
    set(qt_abi "win64_msvc${msvc_year}_64")
  else ()
    message(FATAL_ERROR "Unknown ABI to use for Qt")
  endif ()

  set(qt_build_stamp "202009071110")

  set(qt_file_name_prefix "${qt_version}-0-${qt_build_stamp}")

  foreach (qt_component IN ITEMS qtbase qtwinextras)
    list(APPEND qt_files
      "${qt_file_name_prefix}${qt_component}-Windows-Windows_10-MSVC${msvc_year}-Windows-Windows_10-X86_64.7z")
  endforeach ()

  set(qt_subdir "${qt_version}/msvc${msvc_year}_64")

  # This URL is only visible inside of Kitware's network.
  # Please use your own Qt Account to obtain these files.
  set(qt_url_root "https://paraview.org/files/dependencies/internal/qt")
  set(qt_url_path "${qt_platform}/desktop/qt5_${qt_version_nodot}/qt.qt5.${qt_version_nodot}.${qt_abi}")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos")
  if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos10.10_package")
    list(APPEND qt_files "qt-5.9.9-macosx10.10-x86_64-arm64.tar.xz")
    set(qt_subdir "qt-5.9.9-macosx10.10-x86_64-arm64")
  else ()
    list(APPEND qt_files "qt-5.15.2-macosx10.13-x86_64-arm64.tar.xz")
    set(qt_subdir "qt-5.15.2-macosx10.13-x86_64-arm64")
  endif()
  set(qt_url_root "https://cmake.org/files/dependencies")
  set(qt_url_path "")
else()
  message(FATAL_ERROR "Unknown OS to use for Qt")
endif ()

# Verify that we know what directory will be extracted.
if (NOT qt_subdir)
  message(FATAL_ERROR
    "The extracted subdirectory is not set")
endif ()

# Build up the path to the file to download.
set(qt_url_prefix "${qt_url_root}/${qt_url_path}")

# Include the file containing the hashes of the files that matter.
include("${CMAKE_CURRENT_LIST_DIR}/download_qt_hashes.cmake")

# Download and extract each file.
foreach (qt_file IN LISTS qt_files)
  # Ensure we have a hash to verify.
  if (NOT DEFINED "${qt_file}_hash")
    message(FATAL_ERROR
      "Unknown hash for ${qt_file}")
  endif ()

  # Download the file.
  file(DOWNLOAD
    "${qt_url_prefix}/${qt_file}"
    ".gitlab/${qt_file}"
    STATUS download_status
    EXPECTED_HASH "SHA256=${${qt_file}_hash}")

  # Check the download status.
  list(GET download_status 0 res)
  if (res)
    list(GET download_status 1 err)
    message(FATAL_ERROR
      "Failed to download ${qt_file}: ${err}")
  endif ()

  # Extract the file.
  execute_process(
    COMMAND
      "${CMAKE_COMMAND}"
      -E tar
      xf "${qt_file}"
    WORKING_DIRECTORY ".gitlab"
    RESULT_VARIABLE res
    ERROR_VARIABLE err
    ERROR_STRIP_TRAILING_WHITESPACE)
  if (res)
    message(FATAL_ERROR
      "Failed to extract ${qt_file}: ${err}")
  endif ()
endforeach ()

# The Windows tarballs have some unfortunate permissions in them that prevent
# deletion when `git clean -ffdx` tries to clean up the directory.
if (qt_platform STREQUAL "windows_x86")
  # Fix permissions.
  file(TO_NATIVE_PATH ".gitlab/${qt_subdir}/*.*" native_qt_dir)
  execute_process(
    # Remove any read-only flags that aren't affected by `icacls`.
    COMMAND
      attrib
      -r # Remove readonly flag
      "${native_qt_dir}"
      /d # Treat as a directory
      /s # Recursive
      /l # Don't dereference symlinks
    RESULT_VARIABLE res
    ERROR_VARIABLE err
    ERROR_STRIP_TRAILING_WHITESPACE)
  if (res)
    message(FATAL_ERROR
      "Failed to fix remove read-only flags in ${qt_file}: ${err}")
  endif ()
endif ()

# Move to a predictable prefix.
file(RENAME
  ".gitlab/${qt_subdir}"
  ".gitlab/qt")
if(qt_host_subdir)
  file(RENAME
    ".gitlab/${qt_host_subdir}"
    ".gitlab/qt-host")
endif()

cmake_minimum_required(VERSION 3.29)

set(qt_tar_workdir ".gitlab")

# Files needed to download.
set(qt_files)
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows.*package")
  set(qt_url_root "https://cmake.org/files/dependencies/qt")
  set(qt_url_path "")
  if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows_x86_64_package")
    list(APPEND qt_files "qt-6.10.1-win-x86_64-msvc_v145-1.zip")
    set(qt_subdir "qt-6.10.1-win-x86_64-msvc_v145-1")
  elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows_i386_package")
    list(APPEND qt_files "qt-6.10.1-win-i386-msvc_v145-1.zip")
    set(qt_subdir "qt-6.10.1-win-i386-msvc_v145-1")
  elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows_arm64_package")
    list(APPEND qt_files "qt-6.10.1-win-arm64-msvc_v145-1.zip")
    set(qt_subdir "qt-6.10.1-win-arm64-msvc_v145-1")
    list(APPEND qt_files "qt-6.10.1-win-x86_64-msvc_v145-1.zip")
    set(qt_host_subdir "qt-6.10.1-win-x86_64-msvc_v145-1")
  else ()
    message(FATAL_ERROR "Unknown arch to use for Qt")
  endif()
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows")
  # This URL is only visible inside of Kitware's network.
  # Please use your own Qt Account to obtain these files.
  set(qt_url_root "https://paraview.org/files/dependencies/internal/qt")
  set(qt_url_path "windows_x86/desktop/qt6_693/qt6_693/qt.qt6.693.win64_msvc2022_64")
  list(APPEND qt_files "6.9.3-0-202509261208qtbase-Windows-Windows_11_23H2-MSVC2022-Windows-Windows_11_23H2-X86_64.7z")
  set(qt_subdir "qt-extract")
  set(qt_tar_workdir ".gitlab/${qt_subdir}")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos")
  if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos10.10_package")
    set(qt_url_root "https://cmake.org/files/dependencies/qt")
    set(qt_url_path "")
    list(APPEND qt_files "qt-5.9.9-macosx10.10-x86_64-arm64.tar.xz")
    set(qt_subdir "qt-5.9.9-macosx10.10-x86_64-arm64")
  else ()
    # This URL is only visible inside of Kitware's network.
    # Please use your own Qt Account to obtain these files.
    set(qt_url_root "https://www.paraview.org/files/dependencies/internal/qt")
    set(qt_url_path "mac_x64/desktop/qt6_693/qt6_693/qt.qt6.693.clang_64")
    list(APPEND qt_files "6.9.3-0-202509261207qtbase-MacOS-MacOS_15-Clang-MacOS-MacOS_15-X86_64-ARM64.7z")
    set(qt_subdir "qt-extract")
    set(qt_tar_workdir ".gitlab/${qt_subdir}")
  endif()
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
  file(MAKE_DIRECTORY "${qt_tar_workdir}")
  execute_process(
    COMMAND
      "${CMAKE_COMMAND}"
      -E tar
      xf "${CMAKE_CURRENT_SOURCE_DIR}/.gitlab/${qt_file}"
    WORKING_DIRECTORY "${qt_tar_workdir}"
    RESULT_VARIABLE res
    ERROR_VARIABLE err
    ERROR_STRIP_TRAILING_WHITESPACE)
  if (res)
    message(FATAL_ERROR
      "Failed to extract ${qt_file}: ${err}")
  endif ()
  file(REMOVE "${qt_file}")
endforeach ()

# Move to a predictable prefix.
file(RENAME
  ".gitlab/${qt_subdir}"
  ".gitlab/qt")
if(qt_host_subdir)
  file(RENAME
    ".gitlab/${qt_host_subdir}"
    ".gitlab/qt-host")
endif()

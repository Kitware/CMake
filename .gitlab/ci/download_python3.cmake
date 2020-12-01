cmake_minimum_required(VERSION 3.17)

set(version "3.8.6")
set(sha256sum "376e18eef7e3ea467f0e3af041b01fc7e2f12855506c2ab2653ceb5e0951212e")
set(dirname "python-${version}-embed-win-x86_64")
set(tarball "${dirname}.tar.xz")

# Download the file.
file(DOWNLOAD
  "https://cmake.org/files/dependencies/${tarball}"
  ".gitlab/${tarball}"
  STATUS download_status
  EXPECTED_HASH "SHA256=${sha256sum}")

# Check the download status.
list(GET download_status 0 res)
if (res)
  list(GET download_status 1 err)
  message(FATAL_ERROR
    "Failed to download ${tarball}: ${err}")
endif ()

# Extract the file.
execute_process(
  COMMAND
    "${CMAKE_COMMAND}"
    -E tar
    xzf "${tarball}"
  WORKING_DIRECTORY ".gitlab"
  RESULT_VARIABLE res
  ERROR_VARIABLE err
  ERROR_STRIP_TRAILING_WHITESPACE)
if (res)
  message(FATAL_ERROR
    "Failed to extract ${tarball}: ${err}")
endif ()

# Move to a predictable directory.
file(RENAME
  ".gitlab/${dirname}"
  ".gitlab/python3")

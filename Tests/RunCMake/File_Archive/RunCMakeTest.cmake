include(RunCMake)

run_cmake(7zip)
run_cmake(7zip-none)
run_cmake(7zip-bz2)
run_cmake(7zip-gz)
run_cmake(7zip-lzma)
run_cmake(7zip-lzma2)
run_cmake(7zip-ppmd)
run_cmake(gnutar)
run_cmake(gnutar-gz)
run_cmake(gnutar-deflate)
run_cmake(pax)
run_cmake(pax-lzma)
run_cmake(pax-lzma2)
run_cmake(pax-xz)
run_cmake(pax-zstd)
run_cmake(paxr)
run_cmake(paxr-bz2)
run_cmake(zip)
run_cmake(zip-none)
run_cmake(zip-deflate)

run_cmake(working-directory)

# Check the THREADS option
run_cmake(argument-validation-threads)
run_cmake(threads-bz2)
run_cmake(threads-gz)
run_cmake(threads-xz)
run_cmake(threads-zstd)

# Encoding tests.  These rely on UTF-8 encoding of our test sources.
if(NOT DEFINED CMake_TEST_LOCALE_CHARSET)
  cmake_host_system_information(RESULT CMake_TEST_LOCALE_CHARSET QUERY LOCALE_CHARSET)
  message(STATUS "Detected LOCALE_CHARSET '${CMake_TEST_LOCALE_CHARSET}'")
endif()

if(CMake_TEST_LOCALE_CHARSET STREQUAL "UTF-8")
  if(CMAKE_HOST_WIN32)
    # The OEM encoding cannot represent our UTF-8 test paths.
    run_cmake(encoding-oem-gnutar-fails)
    run_cmake(encoding-oem-zip-fails)
    run_cmake(CMP0213-OLD-fails)
    run_cmake(CMP0213-WARN-fails)
  else()
    run_cmake(encoding-oem-gnutar)
    run_cmake(encoding-oem-zip)
    run_cmake(CMP0213-OLD)
    run_cmake(CMP0213-WARN)
  endif()

  run_cmake(CMP0213-NEW)

  run_cmake(encoding-oem-7zip)
  run_cmake(encoding-oem-pax)
  run_cmake(encoding-oem-paxr)

  run_cmake(encoding-utf-8-7zip)
  run_cmake(encoding-utf-8-gnutar)
  run_cmake(encoding-utf-8-pax)
  run_cmake(encoding-utf-8-paxr)
  run_cmake(encoding-utf-8-zip)
else()
  message(STATUS "encoding-* - SKIPPED due to non-UTF-8 locale")
endif()

# Extracting only selected files or directories
run_cmake(zip-filtered)

run_cmake(create-missing-args)
run_cmake(extract-missing-args)

run_cmake(unsupported-format)
run_cmake(zip-with-bad-compression)
run_cmake(gnutar-with-bad-compression)

run_cmake(unsupported-compression-level)
run_cmake(argument-validation-compression-level-1)
run_cmake(argument-validation-compression-level-2)
run_cmake(7zip-bz2-compression-level)
run_cmake(7zip-lzma-compression-level)
run_cmake(7zip-xz-compression-level)
run_cmake(7zip-ppmd-compression-level)
run_cmake(gnutar-gz-compression-level)
run_cmake(pax-xz-compression-level)
run_cmake(pax-zstd-compression-level)
run_cmake(paxr-bz2-compression-level)
run_cmake(zip-deflate-compression-level)

# Security: Test path traversal protection
if(Python_EXECUTABLE)
  run_cmake_script(path-absolute -DPython_EXECUTABLE=${Python_EXECUTABLE})
  run_cmake_script(path-traversal -DPython_EXECUTABLE=${Python_EXECUTABLE})
endif()

run_cmake(extract-through-symlink)

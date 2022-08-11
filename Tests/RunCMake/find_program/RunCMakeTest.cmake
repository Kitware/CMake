include(RunCMake)

run_cmake(EnvAndHints)
run_cmake(DirsPerName)
run_cmake(NamesPerDir)
run_cmake(RelAndAbsPath)
run_cmake(Required)
run_cmake(NO_CACHE)
run_cmake(IgnorePrefixPath)
run_cmake(REGISTRY_VIEW-no-view)
run_cmake(REGISTRY_VIEW-wrong-view)

if(CMAKE_SYSTEM_NAME MATCHES "^(Windows|CYGWIN|MSYS)$")
  run_cmake(WindowsCom)
  run_cmake(WindowsExe)
else()
  # test non readable file only if not root
  execute_process(
    COMMAND id -u $ENV{USER}
    OUTPUT_VARIABLE uid
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT "${uid}" STREQUAL "0")
    run_cmake(CMP0109-WARN)
    run_cmake(CMP0109-OLD)
    run_cmake(CMP0109-NEW)
  endif()
endif()

if(APPLE)
  run_cmake(BundleSpaceInName)
endif()

run_cmake_with_options(EnvAndHintsDebugVar --debug-find-var=PROG)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  # Tests using the Windows registry
  find_program(REG NAMES "reg.exe" NO_CACHE)
  if (REG)
    ## check host architecture
    cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "HKCU" SUBKEYS VIEW 64 ERROR_VARIABLE status)
    if (status STREQUAL "")
      set(ARCH "64bit")
    else()
      set(ARCH "32bit")
    endif()

    # crete some entries in the registry
    cmake_path(CONVERT "${RunCMake_SOURCE_DIR}/registry_host${ARCH}.reg" TO_NATIVE_PATH_LIST registry_data)
    execute_process(COMMAND "${REG}" import "${registry_data}" OUTPUT_QUIET ERROR_QUIET)

    run_cmake_with_options(Registry-query -DARCH=${ARCH})

    # clean-up registry
    execute_process(COMMAND "${REG}" delete "HKCU\\SOFTWARE\\Classes\\CLSID\\CMake-Tests\\find_program" /f OUTPUT_QUIET ERROR_QUIET)
    if (ARCH STREQUAL "64bit")
      execute_process(COMMAND "${REG}" delete "HKCU\\SOFTWARE\\Classes\\WOW6432Node\\CLSID\\CMake-Tests\\find_program" /f OUTPUT_QUIET ERROR_QUIET)
    endif()
  endif()
endif()

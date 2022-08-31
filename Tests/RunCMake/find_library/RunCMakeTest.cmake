include(RunCMake)

run_cmake(Created)
run_cmake(FromPrefixPath)
run_cmake(FromPATHEnv)
run_cmake_with_options(IgnoreInstallPrefix "-DCMAKE_INSTALL_PREFIX=${RunCMake_BINARY_DIR}/IgnoreInstallPrefix-build/")
run_cmake_with_options(IgnoreStagingPrefix "-DCMAKE_STAGING_PREFIX=${RunCMake_BINARY_DIR}/IgnoreStagingPrefix-build/")
run_cmake_with_options(IgnoreStagingAndInstallPrefix "-DCMAKE_STAGING_PREFIX=${RunCMake_BINARY_DIR}/IgnoreStagingAndInstallPrefix-build/" "-DCMAKE_INSTALL_PREFIX=${RunCMake_BINARY_DIR}/IgnoreStagingAndInstallPrefix-build/")
if(UNIX AND NOT CYGWIN)
  run_cmake(LibArchLink)
  run_cmake(LibSymLink)
endif()
run_cmake(PrefixInPATH)
run_cmake(Required)
run_cmake(NO_CACHE)
run_cmake(REGISTRY_VIEW-no-view)
run_cmake(REGISTRY_VIEW-wrong-view)
run_cmake(VALIDATOR-no-function)
run_cmake(VALIDATOR-undefined-function)
run_cmake(VALIDATOR-specify-macro)
run_cmake(VALIDATOR)

run_cmake_script(FromScriptMode "-DTEMP_DIR=${RunCMake_BINARY_DIR}/FromScriptMode-temp")

run_cmake_with_options(FromPATHEnvDebugVar --debug-find-var=CREATED_LIBRARY)

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
    execute_process(COMMAND "${REG}" delete "HKCU\\SOFTWARE\\Classes\\CLSID\\CMake-Tests\\find_library" /f OUTPUT_QUIET ERROR_QUIET)
    if (ARCH STREQUAL "64bit")
      execute_process(COMMAND "${REG}" delete "HKCU\\SOFTWARE\\Classes\\WOW6432Node\\CLSID\\CMake-Tests\\find_library" /f OUTPUT_QUIET ERROR_QUIET)
    endif()
  endif()
endif()

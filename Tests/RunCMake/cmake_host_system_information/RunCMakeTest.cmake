include(RunCMake)

run_cmake(BadArg1)
run_cmake(BadArg2)
run_cmake(BadArg3)

run_cmake(QueryList)
run_cmake(QueryKeys)

run_cmake(UnitTest)
run_cmake(Exherbo)
run_cmake(Ubuntu)

run_cmake(CentOS6)
run_cmake(Debian6)

run_cmake(UserFallbackScript)

if(RunCMake_GENERATOR MATCHES "^Visual Studio " AND NOT RunCMake_GENERATOR STREQUAL "Visual Studio 9 2008")
  run_cmake(VsMSBuild)
else()
  run_cmake(VsMSBuildMissing)
endif()

# WINDOWS_REGISTRY tests
run_cmake(Registry_NoArgs)
run_cmake(Registry_BadQuery1)
run_cmake(Registry_BadQuery2)
run_cmake(Registry_BadView1)
run_cmake(Registry_BadView2)
run_cmake(Registry_BadView3)
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  run_cmake(Registry_BadKey1)
  run_cmake(Registry_BadKey2)

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

    run_cmake_with_options(Registry_Query -DARCH=${ARCH})

    # clean-up registry
    execute_process(COMMAND "${REG}" delete "HKCU\\SOFTWARE\\Classes\\CLSID\\CMake-Tests\\chsi-registry" /f OUTPUT_QUIET ERROR_QUIET)
    if (ARCH STREQUAL "64bit")
      execute_process(COMMAND "${REG}" delete "HKCU\\SOFTWARE\\Classes\\WOW6432Node\\CLSID\\CMake-Tests\\chsi-registry" /f OUTPUT_QUIET ERROR_QUIET)
    endif()
  endif()
endif()

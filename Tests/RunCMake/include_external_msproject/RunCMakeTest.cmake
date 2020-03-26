include(RunCMake)
include(${CMAKE_CURRENT_LIST_DIR}/check_utils.cmake)

run_cmake(CustomGuid)
run_cmake(CustomTypePlatform)
run_cmake(CustomGuidTypePlatform)
run_cmake(CustomConfig)

if(RunCMake_GENERATOR MATCHES "Visual Studio ([^9]|9[0-9])")
  run_cmake(SkipGetTargetFrameworkProperties)
endif()

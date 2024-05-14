include(RunCMake)
include(${CMAKE_CURRENT_LIST_DIR}/check_utils.cmake)

run_cmake(CustomGuid)
run_cmake(CustomTypePlatform)
run_cmake(CustomGuidTypePlatform)
run_cmake(CustomConfig)

if(RunCMake_GENERATOR MATCHES "Visual Studio")
  run_cmake(SkipGetTargetFrameworkProperties)
  run_cmake(VSCSharpReference)
endif()

if(RunCMake_GENERATOR MATCHES "^Visual Studio (1[6-9]|[2-9][0-9])"
    AND NOT RunCMake_GENERATOR_TOOLSET MATCHES "^(v80|v90|v100|v110|v120)$")
  function(run_VSCSharpOnlyProject)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/VSCSharpOnlyProject-build)
    run_cmake(VSCSharpOnlyProject)
    set(RunCMake_TEST_NO_CLEAN 1)
    set(build_flags /restore)
    run_cmake_command(VSCSharpOnlyProject-build ${CMAKE_COMMAND} --build . -- ${build_flags})
  endfunction()
  run_VSCSharpOnlyProject()
endif()

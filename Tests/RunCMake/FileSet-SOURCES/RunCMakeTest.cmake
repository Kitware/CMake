
include(RunCMake)

function(run_and_build test)
  if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(config_options "-DCMAKE_CONFIGURATION_TYPES=Debug")
  else()
    set(config_options -DCMAKE_BUILD_TYPE=Debug)
  endif()
  set(RunCMake_TEST_OPTIONS ${config_options})
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${test}-build")
  run_cmake_with_options(${test})
  set(RunCMake_TEST_NO_CLEAN 1)
  if(ARGN)
    foreach(target IN LISTS ARGN)
    run_cmake_command(${test}-${target} ${CMAKE_COMMAND} --build . --config Debug --target ${target} --verbose)
    endforeach()
  else()
    run_cmake_command(${test}-build ${CMAKE_COMMAND} --build . --config Debug --verbose)
  endif()
endfunction()

function(run_and_check test)
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  run_and_build(${test} ${ARGN})
endfunction()


run_cmake(FileNoExist)
run_cmake(TargetProperties)

run_and_build(FileSetProperties)
run_and_build(CustomCommandInput)
run_and_build(IncludeDirectoriesOrder)
run_and_build(FileSetTransitivity)
run_and_check(CompileOptionsOrder)

# Some environments are excluded because they are not able to honor verbose mode
if ((RunCMake_GENERATOR MATCHES "Makefiles|Ninja|Xcode"
    OR (RunCMake_GENERATOR MATCHES "Visual Studio" AND MSVC_VERSION GREATER_EQUAL "1600"))
    AND NOT CMAKE_C_COMPILER_ID STREQUAL "Intel")
  run_and_check(FileSetProperties2 lib1 main)
endif()

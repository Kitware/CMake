include(RunCMake)

run_cmake(Inspect)
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

function(run_tutorial_step name)
  if(ARGV1 STREQUAL "TUTORIALPROJECT_SUBDIR")
    set(RunCMake_TEST_SOURCE_DIR ${Tutorial_SOURCE_DIR}/${name}/TutorialProject)
  else()
    set(RunCMake_TEST_SOURCE_DIR ${Tutorial_SOURCE_DIR}/${name})
  endif()

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}/build)
  set(config Release)

  if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(exe ${RunCMake_TEST_BINARY_DIR}/Tutorial/${config}/Tutorial)
  else()
    set(exe ${RunCMake_TEST_BINARY_DIR}/Tutorial/Tutorial)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=${config})
  endif()

  if(ARGV1 STREQUAL "NO_PRESET")
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_CXX_STANDARD=11)
  else()
    list(APPEND RunCMake_TEST_OPTIONS --preset tutorial)
  endif()

  if(ARGV2 STREQUAL "SETUP_CMAKE_PREFIX")
    list(APPEND RunCMake_TEST_OPTIONS
      "-DCMAKE_PREFIX_PATH=${Tutorial_SOURCE_DIR}/${name}/install\;${RunCMake_BINARY_DIR}/${name}/install"
    )
  endif()

  list(APPEND RunCMake_TEST_OPTIONS -B ${RunCMake_TEST_BINARY_DIR})

  run_cmake(${name}-configure)

  unset(RunCMake_TEST_OPTIONS)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build . --config ${config})

  if(name STREQUAL "Step0")
    return()
  endif()

  unset(RunCMake_TEST_OUTPUT_MERGE)
  set(RunCMake-stdout-file ${name}-run-stdout.txt)
  run_cmake_command(${name}-run ${exe} 25)

endfunction()

run_tutorial_step(Step0 NO_PRESET)
run_tutorial_step(Step3 NO_PRESET)

if(NOT can_build_cxx20_tutorial)
  return()
endif()

foreach(num RANGE 4 9)
  run_tutorial_step(Step${num})
endforeach()

run_tutorial_step(Step10 TUTORIALPROJECT_SUBDIR)

function(install_simpletest name)
  set(RunCMake_TEST_SOURCE_DIR ${Tutorial_SOURCE_DIR}/${name}/SimpleTest)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}/build-simpletest)
  set(config Release)

  list(APPEND RunCMake_TEST_OPTIONS
    -B ${RunCMake_TEST_BINARY_DIR}
    -DCMAKE_INSTALL_PREFIX=${RunCMake_BINARY_DIR}/${name}/install
    --preset tutorial
  )
  run_cmake(${name}-simpletest-configure)

  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${name}-simpletest-install ${CMAKE_COMMAND} --install ${RunCMake_TEST_BINARY_DIR} --config ${config})
endfunction()

install_simpletest(Step11)
run_tutorial_step(Step11 TUTORIALPROJECT_SUBDIR SETUP_CMAKE_PREFIX)

install_simpletest(Complete)
run_tutorial_step(Complete TUTORIALPROJECT_SUBDIR SETUP_CMAKE_PREFIX)

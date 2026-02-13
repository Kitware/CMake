include(RunCMake)

function(build_project test)
  set(RunCMake_TEST_NO_CLEAN FALSE)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()

  run_cmake(${test})

  set(RunCMake_TEST_NO_CLEAN TRUE)
  run_cmake_command(${test}-build ${CMAKE_COMMAND} --build . --config Release)
  run_cmake_command(${test}-install ${CMAKE_COMMAND} --install . --config Release)
endfunction()

build_project(TestLibrary)
build_project(TestExecutable)

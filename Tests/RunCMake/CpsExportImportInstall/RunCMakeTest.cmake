include(RunCMake)

set(RunCMake_TEST_OPTIONS
  -Wno-dev
  "-DCMAKE_EXPERIMENTAL_EXPORT_PACKAGE_INFO:STRING=b80be207-778e-46ba-8080-b23bba22639e"
  "-DCMAKE_EXPERIMENTAL_FIND_CPS_PACKAGES:STRING=e82e467b-f997-4464-8ace-b00808fff261"
  )

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

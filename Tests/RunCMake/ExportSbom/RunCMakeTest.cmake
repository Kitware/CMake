include(RunCMake)

set(common_test_options
  -Wno-dev
  "-DCMAKE_EXPERIMENTAL_GENERATE_SBOM:STRING=ca494ed3-b261-4205-a01f-603c95e4cae0"
  "-DCMAKE_EXPERIMENTAL_FIND_CPS_PACKAGES:STRING=e82e467b-f997-4464-8ace-b00808fff261"
  "-DCMAKE_EXPERIMENTAL_EXPORT_PACKAGE_INFO:STRING=b80be207-778e-46ba-8080-b23bba22639e"
)

function(run_cmake_install test)
  set(extra_options ${ARGN})
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_INSTALL_DIR ${RunCMake_BINARY_DIR}/${test}-install)
  set(RunCMake_TEST_OPTIONS ${common_test_options} ${extra_options})
  list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_INSTALL_PREFIX=${RunCMake_TEST_INSTALL_DIR})
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=DEBUG)
  endif()

  run_cmake(${test})
  set(RunCMake_TEST_NO_CLEAN TRUE)
  run_cmake_command(${test}-build ${CMAKE_COMMAND} --build . --config Debug)
  run_cmake_command(${test}-install ${CMAKE_COMMAND} --install . --config Debug)
endfunction()

run_cmake_install(ApplicationTarget)
run_cmake_install(InterfaceTarget)
run_cmake_install(SharedTarget)
run_cmake_install(Requirements)

run_cmake_install(MissingPackageNamespace)
run_cmake_install(ReferencesNonExportedTarget)

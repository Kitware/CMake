include(RunCMake)

set(common_test_options
  -Wno-author
  "-DCMAKE_EXPERIMENTAL_GENERATE_SBOM:STRING=2d856d6d-53e8-488b-a17f-d486d2cac317"
)

function(run_cmake_error test)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_OPTIONS ${common_test_options})
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=DEBUG)
  endif()
  run_cmake(${test})
endfunction()

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
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${test}-build ${CMAKE_COMMAND} --build . --config Debug)
  unset(RunCMake_TEST_OUTPUT_MERGE)
  run_cmake_command(${test}-install ${CMAKE_COMMAND} --install . --config Debug)
endfunction()

run_cmake_install(ApplicationTarget)
run_cmake_install(InstallExportPlusSbomSameSet)
run_cmake_install(InterfaceTarget)
run_cmake_install(SharedTarget)
run_cmake_install(Requirements)
run_cmake_install(SbomNamespaceFallback)
run_cmake_install(MultiSetSingleSbom)
run_cmake_install(TargetInMultipleSets)
run_cmake_install(EmptyNamespaceFallback -Wauthor)
run_cmake_install(SbomNamespaceAmbiguity -Wauthor)

run_cmake_install(IgnoresInterfaceDirs)
run_cmake_install(MissingPackageNamespace)
run_cmake_install(ProjectMetadata)

run_cmake_error(ReferencesNonExportedTarget)
run_cmake_error(MultiNamespaceAmbiguity)
run_cmake_error(MultiSetAmbiguity)
run_cmake_error(DuplicateSbom)

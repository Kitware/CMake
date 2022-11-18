cmake_minimum_required(VERSION 3.23)
include(RunCMake)

function(run_ExportImport_test case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-export-build)
  set(CMAKE_INSTALL_PREFIX ${RunCMake_TEST_BINARY_DIR}/root)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  run_cmake(${case}-export)
  unset(RunCMake_TEST_OPTIONS)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-export-build ${CMAKE_COMMAND} --build . --config Debug)
  run_cmake_command(${case}-export-install ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DBUILD_TYPE=Debug -P cmake_install.cmake)
  unset(RunCMake_TEST_NO_CLEAN)

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-import-build)
  run_cmake_with_options(${case}-import
    -Dfoo_DIR=${CMAKE_INSTALL_PREFIX}/lib/cmake/foo
    -Dbar_DIR=${CMAKE_INSTALL_PREFIX}/lib/cmake/bar
    )
endfunction()

run_ExportImport_test(SharedDep)

function(run_ExportImportBuildInstall_test case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-export-build)
  set(CMAKE_INSTALL_PREFIX ${RunCMake_TEST_BINARY_DIR}/root)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  run_cmake(${case}-export)
  unset(RunCMake_TEST_OPTIONS)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-export-build ${CMAKE_COMMAND} --build . --config Debug)
  run_cmake_command(${case}-export-install ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DBUILD_TYPE=Debug -P cmake_install.cmake)
  unset(RunCMake_TEST_NO_CLEAN)

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-import-build)
  run_cmake_with_options(${case}-import
    -Dbuild_DIR=${RunCMake_BINARY_DIR}/${case}-export-build
    -Dinstall_DIR=${CMAKE_INSTALL_PREFIX}/lib/cmake/install
    )
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-import-build ${CMAKE_COMMAND} --build . --config Debug)
  unset(RunCMake_TEST_NO_CLEAN)
endfunction()

run_ExportImportBuildInstall_test(BuildInstallInterfaceGenex)

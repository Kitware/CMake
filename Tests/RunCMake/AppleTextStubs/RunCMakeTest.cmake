include(RunCMake)

function(build_project test)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()
  run_cmake(${test})
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)

  run_cmake_command(${test}-build ${CMAKE_COMMAND} --build . --config Release)
  if ("${ARGC}" GREATER "1")
    # custom install step
    cmake_language(CALL ${ARGV1})
  else()
    run_cmake_command(${test}-install ${CMAKE_COMMAND} --install . --config Release)
  endif()
endfunction()

build_project(Simple)
build_project(Framework)

function(LibraryWithOutputs-run)
  run_cmake_command(${test}-run ${CMAKE_COMMAND} --build . --target run --config Release)
endfunction()

build_project(LibraryWithOutputs LibraryWithOutputs-run)


function(LibraryWithVersions-install)
  run_cmake_command(LibraryWithVersions-install-component-lib3 ${CMAKE_COMMAND} --install . --config Release --component lib3)
  run_cmake_command(LibraryWithVersions-install-component-lib4 ${CMAKE_COMMAND} --install . --config Release --component lib4)
  run_cmake_command(LibraryWithVersions-install-components-dev4 ${CMAKE_COMMAND} --install . --config Release --component dev4)
  run_cmake_command(LibraryWithVersions-install ${CMAKE_COMMAND} --install . --config Release  --component default)
endfunction()

build_project(LibraryWithVersions LibraryWithVersions-install)


function(build_ExportImport_project test)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-export-build)
  set(CMAKE_INSTALL_PREFIX ${RunCMake_TEST_BINARY_DIR}/root)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()
  run_cmake(${test}-export)
  unset(RunCMake_TEST_OPTIONS)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-export-build ${CMAKE_COMMAND} --build . --config Release)
  run_cmake_command(${test}-export-install ${CMAKE_COMMAND} --install . --prefix ${CMAKE_INSTALL_PREFIX} --config Release)

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-import-build)
  set (foo_BUILD "${RunCMake_BINARY_DIR}/${test}-export-build")
  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    string (APPEND foo_BUILD "/Release")
  endif()
  run_cmake_with_options(${test}-import -Dfoo_DIR=${CMAKE_INSTALL_PREFIX}/lib/foo
                                        -Dfoo_BUILD=${RunCMake_BINARY_DIR}/${test}-export-build/Release)
  run_cmake_command(${test}-import-build ${CMAKE_COMMAND} --build . --config Release)
endfunction()

build_ExportImport_project(Library)
build_ExportImport_project(Framework)

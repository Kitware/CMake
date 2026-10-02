include(RunCMake)

# Function called in *-check.cmake scripts to check installed files.
function(check_installed expect)
  file(GLOB_RECURSE actual
    LIST_DIRECTORIES TRUE
    RELATIVE ${CMAKE_INSTALL_PREFIX}
    ${CMAKE_INSTALL_PREFIX}/*
    )
  if(actual)
    list(SORT actual)
  endif()
  if(NOT "${actual}" MATCHES "${expect}")
    set(RunCMake_TEST_FAILED "Installed files:
  ${actual}
do not match what we expected:
  ${expect}
in directory:
  ${CMAKE_INSTALL_PREFIX}\n" PARENT_SCOPE)
  endif()
endfunction()

# Functions called in *-check.cmake scripts to check whether preparing for
# installation built the given static library targets.
function(check_prepared)
  foreach(target IN LISTS ARGN)
    if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/lib${target}.a" AND
       NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/${target}.lib" AND
       NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/${target}.l")
      set(RunCMake_TEST_FAILED "Preparation did not build ${target}." PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

function(check_not_prepared)
  foreach(target IN LISTS ARGN)
    file(GLOB_RECURSE artifacts LIST_DIRECTORIES FALSE
      "${RunCMake_TEST_BINARY_DIR}/lib${target}.a"
      "${RunCMake_TEST_BINARY_DIR}/${target}.lib"
      )
    if(artifacts)
      set(RunCMake_TEST_FAILED "Preparation built ${target}: ${artifacts}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

# Configure a test project.  Callers set RunCMake_TEST_NO_CLEAN afterward to
# run further commands in the same build tree.
macro(configure_install_prep test config)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(CMAKE_INSTALL_PREFIX ${RunCMake_BINARY_DIR}/${test}-install)
  file(REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}")
  set(RunCMake_TEST_CONFIG "")
  if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_CONFIG "${config}")
  endif()
  set(RunCMake_TEST_OPTIONS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=${config})
  endif()
  # Some generators issue additional diagnostics.
  set(stderr_variant "")
  if(RunCMake_GENERATOR MATCHES "Make")
    set(stderr_variant make)
  endif()
  if(stderr_variant AND
      EXISTS "${RunCMake_SOURCE_DIR}/${test}-stderr-${stderr_variant}.txt")
    set(RunCMake-stderr-file ${test}-stderr-${stderr_variant}.txt)
  endif()
  run_cmake(${test})
  unset(RunCMake-stderr-file)
  set(RunCMake_TEST_NO_CLEAN TRUE)
endmacro()

# Build a preparation target that is expected not to exist.
function(run_missing_install_prep test prep_target)
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  run_cmake_command(${test}-build ${CMAKE_COMMAND}
    --build . --config Debug --target ${prep_target})
endfunction()

# Build a preparation target and then install the matching component.
function(run_install_prep test prep_target install_component)
  configure_install_prep(${test} Debug)
  run_cmake_command(${test}-prep ${CMAKE_COMMAND}
    --build . --config Debug --target ${prep_target})
  if(EXISTS "${CMAKE_INSTALL_PREFIX}")
    message(SEND_ERROR "${test}: preparation ran installation.")
  endif()

  set(install_args --install . --config Debug)
  if(NOT install_component STREQUAL "ALL")
    list(APPEND install_args --component ${install_component})
  endif()
  set(RunCMake-check-file ${test}-install-check.cmake)
  run_cmake_command(${test}-install ${CMAKE_COMMAND} ${install_args})
endfunction()

block()
  configure_install_prep(InstallPrepGate-ON Debug)
  run_missing_install_prep(InstallPrepGate-ON install_prep/all)
endblock()

block()
  configure_install_prep(NoInstallRules Debug)
  run_missing_install_prep(NoInstallRules install_prep/all)
endblock()

run_install_prep(All install_prep/all ALL)
run_install_prep(Runtime install_prep/Runtime Runtime)
run_install_prep(Development install_prep/Development Development)
run_install_prep(Nested
  install_prep/Nested/Runtime_1.2+dbg Nested/Runtime_1.2+dbg)
run_install_prep(Default install_prep/DefaultComponent DefaultComponent)
run_install_prep(Hidden install_prep/Hidden Hidden)
run_install_prep(Imported install_prep/Imported Imported)
run_install_prep(Documentation install_prep/Documentation Documentation)
run_install_prep(ExcludedSubdirectory
  install_prep/ExcludedSubdirectory ExcludedSubdirectory)
run_install_prep(GeneratedFileSetHeaders install_prep/Headers Headers)
run_install_prep(GeneratedTargetProperties install_prep/Headers Headers)
run_install_prep(GeneratedByOtherTarget install_prep/Headers Headers)
run_install_prep(GeneratedByDependency install_prep/Headers Headers)
run_install_prep(GeneratedSubdirectory install_prep/Headers Headers)
run_install_prep(GeneratedFiles install_prep/Files Files)
run_install_prep(TargetFileArtifact install_prep/Artifacts Artifacts)
if(RunCMake_GENERATOR MATCHES "Ninja")
  run_install_prep(GeneratedFilesByproduct install_prep/Byproduct Byproduct)
else()
  # The byproduct cannot be prepared, which is diagnosed.
  block()
    configure_install_prep(GeneratedFilesByproduct Debug)
  endblock()
endif()

block()
  configure_install_prep(ComponentNames Debug)
  run_cmake_command(ComponentNames-all-build ${CMAKE_COMMAND}
    --build . --config Debug --target install_prep/all)
  if(RunCMake_GENERATOR MATCHES "Make")
    set(RunCMake_TEST_EXPECT_RESULT "[^0]")
    set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  endif()
  run_cmake_command(ComponentNames-colon-build ${CMAKE_COMMAND}
    --build . --config Debug --target install_prep/Colon:Name)
  unset(RunCMake_TEST_EXPECT_RESULT)
  unset(RunCMake_TEST_OUTPUT_MERGE)
  run_missing_install_prep(ComponentNames-dotdot install_prep/Unsafe/../Name)
  run_missing_install_prep(ComponentNames-empty-segment
    install_prep/Empty//Segment)
  run_missing_install_prep(ComponentNames-dot install_prep/./Dot)
endblock()

function(run_configured_install_prep test config)
  configure_install_prep(${test} ${config})
  run_cmake_command(${test}-prep ${CMAKE_COMMAND}
    --build . --config ${config} --target install_prep/Configured)
  if(EXISTS "${CMAKE_INSTALL_PREFIX}")
    message(SEND_ERROR "${test}: preparation ran installation.")
  endif()
  set(RunCMake-check-file ${test}-install-check.cmake)
  run_cmake_command(${test}-install ${CMAKE_COMMAND}
    --install . --config ${config} --component Configured)
endfunction()

run_configured_install_prep(ConfigurationsDebug Debug)
run_configured_install_prep(ConfigurationsRelease Release)

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  block()
    configure_install_prep(ConfigurationsMulti Debug)
    foreach(config IN ITEMS Debug Release)
      set(RunCMake_TEST_CONFIG "${config}")
      run_cmake_command(ConfigurationsMulti-prep-${config} ${CMAKE_COMMAND}
        --build . --config ${config} --target install_prep/Configured)
      run_cmake_command(ConfigurationsMulti-install-${config} ${CMAKE_COMMAND}
        --install . --config ${config} --component Configured)
    endforeach()
  endblock()
endif()

if(CMAKE_HOST_SYSTEM_NAME MATCHES "^(Windows|Linux|Darwin|FreeBSD)$")
  run_install_prep(RuntimeDependencySet
    install_prep/RuntimeDependencySet RuntimeDependencySet)
  run_install_prep(RuntimeDependencies
    install_prep/RuntimeDependencies RuntimeDependencies)
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  run_install_prep(RuntimeDependencyFramework
    install_prep/FrameworkDependencies FrameworkDependencies)
endif()

if(CMAKE_HOST_SYSTEM_NAME MATCHES "^(Linux|Darwin)$")
  run_install_prep(Namelink install_prep/Namelink Namelink)
endif()

if(RunCMake_GENERATOR MATCHES "Make" AND
    CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  block()
    configure_install_prep(MakefilePreinstall Debug)
    run_cmake_command(MakefilePreinstall-prep ${CMAKE_COMMAND}
      --build . --config Debug --target install_prep/Relink)
    run_cmake_command(MakefilePreinstall-run
      ${RunCMake_TEST_BINARY_DIR}/relink)
    set(RunCMake-check-file MakefilePreinstall-install-check.cmake)
    run_cmake_command(MakefilePreinstall-install ${CMAKE_COMMAND}
      --install . --config Debug --component Relink)
  endblock()
endif()

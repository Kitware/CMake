include(RunCMake)

function(run_GEH)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/GEH-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(GEH)
  run_cmake_command(GEH-build ${CMAKE_COMMAND} --build . --config Debug)
  run_cmake_command(GEH-run ${RunCMake_TEST_BINARY_DIR}/GenerateExportHeader)
  run_cmake_command(GEH-incguard-macro-run ${RunCMake_TEST_BINARY_DIR}/test_includeguard_macro)
  run_cmake_command(GEH-incguard-custom-run ${RunCMake_TEST_BINARY_DIR}/test_includeguard_custom)

  file(STRINGS "${RunCMake_TEST_BINARY_DIR}/failure_test_targets"
    failure_test_targets)

  foreach(failure_test_target ${failure_test_targets})
    run_cmake_command(GEH-link-error ${CMAKE_COMMAND}
      --build .
      --config Debug
      --target ${failure_test_target})
  endforeach()

  file(STRINGS "${RunCMake_TEST_BINARY_DIR}/deprecated_msg_targets"
    deprecated_msg_targets)

  # Generators print compiler warnings to either stream.
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  foreach(deprecated_msg_target ${deprecated_msg_targets})
    run_cmake_command(GEH-deprecated-msg ${CMAKE_COMMAND}
      --build .
      --config Debug
      --target ${deprecated_msg_target})
  endforeach()
  unset(RunCMake_TEST_OUTPUT_MERGE)
endfunction()

# remove these flags from the environment if they have been set
# so the tests run the correct env
set(env_cxx_flags $ENV{CXXFLAGS})
if(env_cxx_flags)
  string(REPLACE "-fvisibility=hidden" "" env_cxx_flags "${env_cxx_flags}")
  string(REPLACE "-fvisibility-inlines-hidden" "" env_cxx_flags "${env_cxx_flags}")
  # The deprecation message checks need warnings.
  string(REGEX REPLACE "(^| )-w( |$)" " " env_cxx_flags "${env_cxx_flags}")
  set(ENV{CXXFLAGS} "${env_cxx_flags}")
endif()
run_GEH()

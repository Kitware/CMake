include(RunCMake)

function(run_codegen case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)

  run_cmake(${case})

  set(RunCMake_TEST_NO_CLEAN 1)

  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --target codegen --config Debug)
endfunction()

# Builds codegen target when there are no custom commands marked codegen
run_codegen("no-codegen")
run_codegen("no-codegen-2")

# We don't want codegen to drive parts of the project that are EXCLUDE_FROM_ALL
run_codegen("exclude-from-all")

# Ensures codegen builds minimal build graphs
run_codegen("min-graph-1")
run_codegen("min-graph-2")
run_codegen("min-graph-3")

# Handle specific cases that can affect codegen
run_codegen("add-dependencies")
run_codegen("add-custom-command-depends")
run_codegen("byproducts")
run_codegen("add-dependencies-not-all")
run_codegen("add-dependencies-transitive")
run_codegen("add-dependencies-exclude-from-all")
run_codegen("add-dependencies-subdir")

# Outputs that differ per configuration
block()
  if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS "-DCMAKE_CONFIGURATION_TYPES=Debug\\;Release")
    if(RunCMake_GENERATOR STREQUAL "Ninja Multi-Config")
      list(APPEND RunCMake_TEST_OPTIONS
        -DCMAKE_CROSS_CONFIGS=all
        -DCMAKE_DEFAULT_BUILD_TYPE=Release
      )
    endif()
  else()
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  run_codegen("per-config")

  if(RunCMake_GENERATOR STREQUAL "Ninja Multi-Config")
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/per-config-build)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(per-config-default-build ${CMAKE_COMMAND} --build . --target codegen)
    file(REMOVE_RECURSE
      ${RunCMake_TEST_BINARY_DIR}/Debug
      ${RunCMake_TEST_BINARY_DIR}/Release
    )
    run_cmake_command(per-config-all-build ${CMAKE_COMMAND} --build . --target codegen:all)
  endif()
endblock()

# codegen re-runs commands only when their inputs change
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/rebuild-build)
  run_cmake(rebuild)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(rebuild-build1 ${CMAKE_COMMAND} --build . --target codegen --config Debug)
  # Make the output newer than its input but with different content.
  file(WRITE ${RunCMake_TEST_BINARY_DIR}/generated.h "stale")
  run_cmake_command(rebuild-build2 ${CMAKE_COMMAND} --build . --target codegen --config Debug)
  # Ensure the input is newer than the output.
  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1)
  file(WRITE ${RunCMake_TEST_BINARY_DIR}/input.txt "2")
  run_cmake_command(rebuild-build3 ${CMAKE_COMMAND} --build . --target codegen --config Debug)
endblock()

# Error handling
run_cmake("implicit-depends")
run_cmake("implicit-depends-append-codegen")
run_cmake("append-implicit-depends")
run_cmake("no-output")

# Top-level codegen depends on that of subdirectories.
run_codegen(SubDir)
run_codegen(SubDirExcludeFromAll)

include(RunCMake)

function(run_codegen case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)

  run_cmake(${case})

  set(RunCMake_TEST_NO_CLEAN 1)

  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --target codegen --config Debug)
endfunction()

# Builds codegen target when there are no custom commands marked codegen
run_codegen("no-codegen")

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

# Error handling
run_cmake("implicit-depends")
run_cmake("implicit-depends-append-codegen")
run_cmake("append-implicit-depends")
run_cmake("no-output")

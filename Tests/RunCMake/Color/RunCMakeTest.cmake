include(RunCMake)

unset(ENV{CMAKE_COLOR_DIAGNOSTICS})

function(run_Diag case)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/Diag${case}-build")
  run_cmake_with_options(Diag${case} ${ARGN})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(Diag${case}-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

run_Diag(On -DCMAKE_COLOR_DIAGNOSTICS=ON)
run_Diag(Off -DCMAKE_COLOR_DIAGNOSTICS=OFF)
run_Diag(Default)

# Test with `COLOR` in the ambient environment set to an ANSI-like color
# sequence. `$(COLOR)` is used in the Makefiles generator to control whether or
# not to do such color sequences itself.
set(ENV{COLOR} "[38;2;255;221;255m")
run_Diag(ColorInEnv)

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

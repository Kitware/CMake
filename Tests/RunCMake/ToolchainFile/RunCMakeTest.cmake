include(RunCMake)

function(run_cmake_toolchain t)
  set(RunCMake_TEST_OPTIONS -DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/${t}-toolchain.cmake)
  run_cmake(${t})
endfunction()

run_cmake_toolchain(CallEnableLanguage)
run_cmake_toolchain(CallProject)
run_cmake_toolchain(FlagsInit)
run_cmake_toolchain(LinkFlagsInit)

function(run_IncludeDirectories)
  run_cmake_toolchain(IncludeDirectories)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/IncludeDirectories-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(IncludeDirectories-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()
run_IncludeDirectories()

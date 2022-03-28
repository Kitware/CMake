include(RunCMake)

function(find_compiler lang)
  # Detect the compiler in use in the current environment.
  run_cmake(Find${lang}Compiler)
  # Use the detected compiler
  include(${RunCMake_BINARY_DIR}/Find${lang}Compiler-build/${lang}_comp.cmake)
  if(NOT temp_CMAKE_${lang}_COMPILER)
    message(FATAL_ERROR "FindCompiler provided no compiler!")
  endif()
  # Create a toolchain file
  set(__test_compiler_var CMAKE_${lang}_COMPILER)
  set(__test_compiler "${temp_CMAKE_${lang}_COMPILER}")
  configure_file(${RunCMake_SOURCE_DIR}/toolchain.cmake.in
      ${RunCMake_BINARY_DIR}/Find${lang}Compiler-build/toolchain_${lang}_comp.cmake @ONLY)
endfunction()

function(run_compiler_env lang)
  # Use the correct compiler
  include(${RunCMake_BINARY_DIR}/Find${lang}Compiler-build/${lang}_comp.cmake)

  # Set the compiler
  if(lang STREQUAL "C")
    set(ENV{CC} "'${temp_CMAKE_${lang}_COMPILER}' -DFOO1 -DFOO2")
  else()
    set(ENV{${lang}} "'${temp_CMAKE_${lang}_COMPILER}' -DFOO1 -DFOO2")
  endif()

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${lang}-env-build)
  set(RunCMake_TEST_VARIANT_DESCRIPTION "-env")
  run_cmake(${lang})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${lang}-Build ${CMAKE_COMMAND} --build . ${verbose_args})

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}/CMakeFiles")
  set(RunCMake_TEST_VARIANT_DESCRIPTION "-env-cached")
  run_cmake(${lang})
  run_cmake_command(${lang}-Build ${CMAKE_COMMAND} --build . ${verbose_args})
endfunction()

function(run_compiler_tc lang)
  set(RunCMake_TEST_OPTIONS
      -DCMAKE_TOOLCHAIN_FILE=${RunCMake_BINARY_DIR}/Find${lang}Compiler-build/toolchain_${lang}_comp.cmake)

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${lang}-tc-build)
  set(RunCMake_TEST_VARIANT_DESCRIPTION "-tc")
  run_cmake(${lang})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${lang}-Build ${CMAKE_COMMAND} --build . ${verbose_args})

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}/CMakeFiles")
  set(RunCMake_TEST_VARIANT_DESCRIPTION "-tc-cached")
  run_cmake(${lang})
  run_cmake_command(${lang}-Build ${CMAKE_COMMAND} --build . ${verbose_args})
endfunction()

set(langs C CXX)

foreach(lang ${langs})
  find_compiler(${lang})
  run_compiler_env(${lang})
  run_compiler_tc(${lang})
endforeach()

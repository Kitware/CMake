
include(RunCMake)

macro(run_cmake_target test subtest target)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-${subtest} ${CMAKE_COMMAND} --build . --target ${target} ${ARGN})

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endmacro()

if (NOT CMAKE_C_COMPILER_ID STREQUAL "Intel")
  # Intel compiler does not reject bad flags or objects!
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()

  run_cmake(LINK_OPTIONS)

  run_cmake_target(LINK_OPTIONS shared LinkOptions_shared --config Release)
  run_cmake_target(LINK_OPTIONS mod LinkOptions_mod --config Release)
  run_cmake_target(LINK_OPTIONS exe LinkOptions_exe --config Release)


  run_cmake(genex_LINK_LANGUAGE)

  run_cmake_target(genex_LINK_LANGUAGE shared_c LinkOptions_shared_c --config Release)
  run_cmake_target(genex_LINK_LANGUAGE shared_cxx LinkOptions_shared_cxx --config Release)
  run_cmake_target(genex_LINK_LANGUAGE mod LinkOptions_mod --config Release)
  run_cmake_target(genex_LINK_LANGUAGE exe LinkOptions_exe --config Release)

  run_cmake(genex_LINK_LANG_AND_ID)

  run_cmake_target(genex_LINK_LANG_AND_ID shared_c LinkOptions_shared_c --config Release)
  run_cmake_target(genex_LINK_LANG_AND_ID shared_cxx LinkOptions_shared_cxx --config Release)
  run_cmake_target(genex_LINK_LANG_AND_ID mod LinkOptions_mod --config Release)
  run_cmake_target(genex_LINK_LANG_AND_ID exe LinkOptions_exe --config Release)

  unset(RunCMake_TEST_OPTIONS)
  unset(RunCMake_TEST_OUTPUT_MERGE)
endif()

run_cmake(bad_SHELL_usage)

if(RunCMake_GENERATOR MATCHES "(Ninja|Makefile)")
  run_cmake(LINKER_expansion)
  run_cmake_target(LINKER_expansion build all)

  run_cmake(LINKER_SHELL_expansion)
  run_cmake_target(LINKER_SHELL_expansion build all)
endif()

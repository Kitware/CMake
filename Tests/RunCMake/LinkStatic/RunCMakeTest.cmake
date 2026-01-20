include(RunCMake)

run_cmake(LINK_SEARCH_STATIC)


macro(run_cmake_target test subtest target)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(RunCMake_GENERATOR STREQUAL "Borland Makefiles")
    set(RunCMake_TEST_EXPECT_RESULT .)
  endif()
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

  run_cmake(STATIC_LIBRARY_OPTIONS)

  run_cmake_target(STATIC_LIBRARY_OPTIONS basic StaticLinkOptions)
  run_cmake_target(STATIC_LIBRARY_OPTIONS genex StaticLinkOptions_genex --config Release)
  run_cmake_target(STATIC_LIBRARY_OPTIONS shared SharedLinkOptions)

  run_cmake(STATIC_LIBRARY_FLAGS)
  run_cmake_target(STATIC_LIBRARY_FLAGS basic StaticLinkFlags)
  run_cmake_target(STATIC_LIBRARY_FLAGS config StaticLinkFlags_config --config Release)
  run_cmake_target(STATIC_LIBRARY_FLAGS shared SharedLinkFlags)

  run_cmake(CMAKE_STATIC_LINKER_FLAGS)
  run_cmake_target(CMAKE_STATIC_LINKER_FLAGS basic CMakeStaticLinkerFlags)
  run_cmake_target(CMAKE_STATIC_LINKER_FLAGS shared SharedCMakeStaticLinkerFlags)

  run_cmake(CMAKE_STATIC_LINKER_FLAGS_CONFIG)
  run_cmake_target(CMAKE_STATIC_LINKER_FLAGS_CONFIG basic CMakeStaticLinkerFlags_config --config Release)
  run_cmake_target(CMAKE_STATIC_LINKER_FLAGS_CONFIG shared SharedCMakeStaticLinkerFlags_config --config Release)

  unset(RunCMake_TEST_OPTIONS)
  unset(RunCMake_TEST_OUTPUT_MERGE)
endif()

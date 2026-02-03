include(RunCMake)

macro(run_cmake_target test subtest)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-${subtest}
    ${CMAKE_COMMAND} --build .
    --target LinkFlags_${subtest}
    --verbose
    ${ARGN}
  )

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endmacro()

if (NOT CMAKE_C_COMPILER_ID STREQUAL "Intel")
  # Intel compiler does not reject bad flags or objects!
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()

  run_cmake(LINK_FLAGS)
  run_cmake_target(LINK_FLAGS shared)
  run_cmake_target(LINK_FLAGS mod)
  run_cmake_target(LINK_FLAGS exe)

  run_cmake(LINK_FLAGS_CONFIG)
  run_cmake_target(LINK_FLAGS_CONFIG shared --config Release)
  run_cmake_target(LINK_FLAGS_CONFIG mod --config Release)
  run_cmake_target(LINK_FLAGS_CONFIG exe --config Release)

  run_cmake(CMAKE_LINKER_FLAGS)
  run_cmake_target(CMAKE_LINKER_FLAGS shared)
  run_cmake_target(CMAKE_LINKER_FLAGS mod)
  run_cmake_target(CMAKE_LINKER_FLAGS exe)

  run_cmake(CMAKE_LINKER_FLAGS_CONFIG)
  run_cmake_target(CMAKE_LINKER_FLAGS_CONFIG shared --config Release)
  run_cmake_target(CMAKE_LINKER_FLAGS_CONFIG mod --config Release)
  run_cmake_target(CMAKE_LINKER_FLAGS_CONFIG exe --config Release)

  run_cmake(CMAKE_LANG_LINK_FLAGS-CMP0210-NEW)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS-CMP0210-NEW shared_C)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS-CMP0210-NEW mod_C)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS-CMP0210-NEW exe_C)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS-CMP0210-NEW shared_CXX)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS-CMP0210-NEW mod_CXX)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS-CMP0210-NEW exe_CXX)

  run_cmake(CMAKE_LANG_LINK_FLAGS_CONFIG-CMP0210-NEW)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS_CONFIG-CMP0210-NEW shared_C --config Release)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS_CONFIG-CMP0210-NEW mod_C --config Release)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS_CONFIG-CMP0210-NEW exe_C --config Release)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS_CONFIG-CMP0210-NEW shared_CXX --config Release)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS_CONFIG-CMP0210-NEW mod_CXX --config Release)
  run_cmake_target(CMAKE_LANG_LINK_FLAGS_CONFIG-CMP0210-NEW exe_CXX --config Release)

  if (NOT RunCMake_GENERATOR MATCHES "Visual Studio")
    # CMP0210's OLD behavior never applied to the Visual Studio generators.
    run_cmake(CMAKE_LANG_LINK_FLAGS-CMP0210-OLD)
    run_cmake_target(CMAKE_LANG_LINK_FLAGS-CMP0210-OLD shared)
    run_cmake_target(CMAKE_LANG_LINK_FLAGS-CMP0210-OLD mod)
    run_cmake_target(CMAKE_LANG_LINK_FLAGS-CMP0210-OLD exe)

    run_cmake(CMAKE_LANG_LINK_FLAGS-CMP0210-WARN)
    run_cmake_target(CMAKE_LANG_LINK_FLAGS-CMP0210-WARN shared)
    run_cmake_target(CMAKE_LANG_LINK_FLAGS-CMP0210-WARN exe)
  endif()

  unset(RunCMake_TEST_OPTIONS)
  unset(RunCMake_TEST_OUTPUT_MERGE)
endif()

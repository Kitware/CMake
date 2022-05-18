
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
  set(RunCMake_TEST_OPTIONS -DCMake_TEST_CUDA=${CMake_TEST_CUDA})
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()
  if (RunCMake_GENERATOR MATCHES "Ninja")
    set(VERBOSE -- -v)
  endif()

  run_cmake(LINK_OPTIONS)

  run_cmake_target(LINK_OPTIONS basic LinkOptions)
  run_cmake_target(LINK_OPTIONS interface LinkOptions_consumer)
  run_cmake_target(LINK_OPTIONS interface-static LinkOptions_consumer_static)
  run_cmake_target(LINK_OPTIONS static LinkOptions_static --config Release)
  run_cmake_target(LINK_OPTIONS shared LinkOptions_shared --config Release)
  run_cmake_target(LINK_OPTIONS mod LinkOptions_mod --config Release)
  run_cmake_target(LINK_OPTIONS exe LinkOptions_exe --config Release)


  run_cmake(genex_LINK_LANGUAGE)

  run_cmake_target(genex_LINK_LANGUAGE interface LinkOptions_shared_interface --config Release)
  run_cmake_target(genex_LINK_LANGUAGE shared_c LinkOptions_shared_c --config Release)
  run_cmake_target(genex_LINK_LANGUAGE LINKER_LANGUAGE LinkOptions_shared_cxx --config Release)
  run_cmake_target(genex_LINK_LANGUAGE mod LinkOptions_mod --config Release)
  run_cmake_target(genex_LINK_LANGUAGE exe LinkOptions_exe --config Release)

  run_cmake(genex_LINK_LANG_AND_ID)

  run_cmake_target(genex_LINK_LANG_AND_ID interface LinkOptions_shared_interface --config Release)
  run_cmake_target(genex_LINK_LANG_AND_ID shared_c LinkOptions_shared_c --config Release)
  run_cmake_target(genex_LINK_LANG_AND_ID LINKER_LANGUAGE LinkOptions_shared_cxx --config Release)
  run_cmake_target(genex_LINK_LANG_AND_ID mod LinkOptions_mod --config Release)
  run_cmake_target(genex_LINK_LANG_AND_ID exe LinkOptions_exe --config Release)

  run_cmake(genex_DEVICE_LINK)

  run_cmake_target(genex_DEVICE_LINK interface LinkOptions_shared_interface --config Release)
  run_cmake_target(genex_DEVICE_LINK private LinkOptions_private --config Release)
  if (CMake_TEST_CUDA)
    run_cmake_target(genex_DEVICE_LINK CMP0105_UNSET LinkOptions_CMP0105_UNSET --config Release)
    run_cmake_target(genex_DEVICE_LINK CMP0105_OLD LinkOptions_CMP0105_OLD --config Release)
    run_cmake_target(genex_DEVICE_LINK CMP0105_NEW LinkOptions_CMP0105_NEW --config Release)
    run_cmake_target(genex_DEVICE_LINK device LinkOptions_device --config Release)

    if (RunCMake_GENERATOR MATCHES "(Ninja|Unix Makefiles)")
      run_cmake_target(genex_DEVICE_LINK host_link_options LinkOptions_host_link_options --config Release ${VERBOSE})
    endif()

    run_cmake_target(genex_DEVICE_LINK no_device LinkOptions_no_device --config Release)
  endif()

  unset(RunCMake_TEST_OPTIONS)
  unset(RunCMake_TEST_OUTPUT_MERGE)
endif()

run_cmake(bad_SHELL_usage)

if(RunCMake_GENERATOR MATCHES "(Ninja|Makefile)")
  run_cmake(LINKER_expansion)

  run_cmake_target(LINKER_expansion LINKER linker)
  run_cmake_target(LINKER_expansion LINKER_SHELL linker_shell)
endif()

run_cmake(empty_keyword_args)

if (NOT CMAKE_C_COMPILER_ID STREQUAL "Intel")
  # Intel compiler does not reject bad flags or objects!
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()

  run_cmake(CMP0099-NEW)
  run_cmake_target(CMP0099-NEW basic LinkOptions_exe)


  run_cmake(CMP0099-OLD)
  run_cmake_target(CMP0099-OLD basic LinkOptions_exe)

  unset(RunCMake_TEST_OPTIONS)
  unset(RunCMake_TEST_OUTPUT_MERGE)
endif()


include(RunCMake)

macro(run_cmake_target test subtest target)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-${subtest} ${CMAKE_COMMAND} --build . --target ${target} ${ARGN})

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endmacro()


run_cmake(bad_SHELL_usage)

if(RunCMake_GENERATOR MATCHES "(Ninja|Makefile)")
  run_cmake(LINKER_expansion)

  run_cmake_target(LINKER_expansion LINKER linker)
  run_cmake_target(LINKER_expansion LINKER_SHELL linker_shell)
  run_cmake_target(LINKER_expansion LINKER_CONSUMER linker_consumer)
endif()

# Some environments are excluded because they are not able to honor verbose mode
if (RunCMake_GENERATOR MATCHES "Makefiles|Ninja|Xcode|Visual Studio"
    AND NOT CMAKE_C_COMPILER_ID STREQUAL "Intel")
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)

  foreach(policy IN ITEMS OLD NEW)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/LINKER_expansion2-CMP0181-${policy}-build)
    run_cmake_with_options(LINKER_expansion2 -DCMP0181=${policy})

    run_cmake_target(LINKER_expansion2-CMP0181-${policy} EXE_LINKER_FLAGS exe_linker_flags --verbose)
    run_cmake_target(LINKER_expansion2-CMP0181-${policy} SHARED_LINKER_FLAGS shared_linker_flags --verbose)
    run_cmake_target(LINKER_expansion2-CMP0181-${policy} MODULE_LINKER_FLAGS module_linker_flags --verbose)


    if (NOT (RunCMake_GENERATOR MATCHES "Visual Studio" OR CMAKE_C_COMPILER_ID MATCHES "Borland|Embarcadero"))
      # Visual Studio generator and Borland, Embarcadero compilers do not use these variables
      set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/LINKER_expansion3-CMP0181-${policy}-build)
      run_cmake_with_options(LINKER_expansion3 -DCMP0181=${policy})

      run_cmake_target(LINKER_expansion3-CMP0181-${policy} C_EXE_CREATE_LINK_FLAGS c_exe_create_link_flags --verbose)
      if (NOT (CMAKE_C_COMPILER_ID STREQUAL "MSVC" OR CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
         # MSVC compiler does not use these variables
        run_cmake_target(LINKER_expansion3-CMP0181-${policy} C_SHARED_CREATE_LINK_FLAGS c_shared_create_link_flags --verbose)
        run_cmake_target(LINKER_expansion3-CMP0181-${policy} C_MODULE_CREATE_LINK_FLAGS c_module_create_link_flags --verbose)
      endif()

      if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/LINKER_expansion4-CMP0181-${policy}-build)
        run_cmake_with_options(LINKER_expansion4 -DCMP0181=${policy})

        run_cmake_target(LINKER_expansion4-CMP0181-${policy} C_CREATE_WIN32_EXE c_create_win32_exe --verbose)
        run_cmake_target(LINKER_expansion4-CMP0181-${policy} C_CREATE_CONSOLE_EXE c_create_console_exe --verbose)
      endif()
    endif()
  endforeach()
endif()

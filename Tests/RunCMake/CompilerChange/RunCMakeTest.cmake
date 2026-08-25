include(RunCMake)

if(UNIX AND "${RunCMake_GENERATOR}" MATCHES "Unix Makefiles|Ninja|FASTBuild")
  # Detect the compiler in use in the current environment.
  run_cmake(FindCompiler)
  include(${RunCMake_BINARY_DIR}/FindCompiler-build/cc.cmake)
  if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "FindCompiler provided no compiler!")
  endif()
  if(NOT IS_ABSOLUTE "${CMAKE_C_COMPILER}")
    message(FATAL_ERROR "FindCompiler provided non-absolute path \"${CMAKE_C_COMPILER}\"!")
  endif()
  if(NOT EXISTS "${CMAKE_C_COMPILER}")
    message(FATAL_ERROR "FindCompiler provided non-existing path \"${CMAKE_C_COMPILER}\"!")
  endif()

  # Now that we have the full compiler path, hide CC.
  unset(ENV{CC})

  # Wrap around the real compiler so we can change the compiler
  # path without changing the underlying compiler.
  set(ccIn ${RunCMake_SOURCE_DIR}/cc.sh.in)
  set(cc1 ${RunCMake_BINARY_DIR}/cc1.sh)
  set(cc2 ${RunCMake_BINARY_DIR}/cc2.sh)
  set(cc3 CMAKE_C_COMPILER-NOTFOUND)
  configure_file(${ccIn} ${cc1} @ONLY)
  configure_file(${ccIn} ${cc2} @ONLY)

  block()
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ChangeCompiler-build)
    run_cmake_with_options(FirstCompiler -DCMAKE_C_COMPILER=${cc1})
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_with_options(SecondCompiler -DCMAKE_C_COMPILER=${cc2})
    run_cmake_with_options(EmptyCompiler -DCMAKE_C_COMPILER=)
  endblock()

  block()
    set(cc1_dot ${RunCMake_BINARY_DIR}/./cc1.sh)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CompilerPath-build)
    set(RunCMake_TEST_VARIANT_DESCRIPTION "-step1")
    run_cmake_with_options(CompilerPath "-DCMAKE_C_COMPILER=${cc1_dot}" -DCACHE_ENTRY=cached)
    set(RunCMake_TEST_NO_CLEAN 1)
    set(RunCMake_TEST_VARIANT_DESCRIPTION "-step2")
    run_cmake_with_options(CompilerPath "-DCMAKE_C_COMPILER=${cc1_dot}")
  endblock()
endif()

# Set up "toolchain" files
file(WRITE "${RunCMake_BINARY_DIR}/foo.cmake" "set(toolchain_var foo)\n")
file(WRITE "${RunCMake_BINARY_DIR}/bar.cmake" "set(toolchain_var bar)\n")
if(UNIX)
  # Can't use file(TO_NATIVE_PATH) because it escapes spaces and cmake_path is
  # 3.20
  set(foo_TOOLCHAIN_FILE "${RunCMake_BINARY_DIR}/foo.cmake")
  set(bar_TOOLCHAIN_FILE "${RunCMake_BINARY_DIR}/bar.cmake")
else()
  set(foo_TOOLCHAIN_FILE "${RunCMake_BINARY_DIR}\\foo.cmake")
  set(bar_TOOLCHAIN_FILE "${RunCMake_BINARY_DIR}\\bar.cmake")
endif()

# New toolchain path comes from file
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ToolchainFromFile-build)
  run_cmake(ToolchainFromFile-step1)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake(ToolchainFromFile-step2)
endblock()

# New toolchain path comes from file but old toolchain has been deleted
file(WRITE "${RunCMake_BINARY_DIR}/baz.cmake" "set(toolchain_var baz)\n")
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ToolchainFromFileDeleted-build)
  run_cmake(ToolchainFromFileDeleted-step1)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake(ToolchainFromFileDeleted-step2)
endblock()

# New toolchain path comes from command line
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ToolchainFromCmdline-build)
  run_cmake_with_options(ToolchainFromCmdline-step1
    "-DCMAKE_TOOLCHAIN_FILE=${foo_TOOLCHAIN_FILE}"
  )
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_with_options(ToolchainFromCmdline-step2
    "-DCMAKE_TOOLCHAIN_FILE=${bar_TOOLCHAIN_FILE}"
  )
endblock()

# Running without toolchain argument does not trigger reconfigure
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ToolchainFromCmdlineOnce-build)
  set(RunCMake-stdout-file
    ${RunCMake_SOURCE_DIR}/ToolchainFromCmdlineOnce-stdout.txt
  )
  run_cmake_with_options(ToolchainFromCmdlineOnce-step1
    "-DCMAKE_TOOLCHAIN_FILE=${foo_TOOLCHAIN_FILE}"
  )
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake(ToolchainFromCmdlineOnce-step2)
endblock()

# New toolchain path comes from a preset file
block()
  set(RunCMake_TEST_SOURCE_DIR ${RunCMake_BINARY_DIR}/ToolchainFromPreset)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ToolchainFromPreset-build)
  run_cmake_with_options(ToolchainFromPreset-step1 --preset default)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_with_options(ToolchainFromPreset-step2 --preset default)
endblock()

# Unchanged relative toolchain file in binary dir does not trigger reconfigure
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/RelativeToolchainBinary)
  run_cmake_with_options(RelativeToolchainBinary-step1
    "-DCMAKE_TOOLCHAIN_FILE=foo.cmake"
  )
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_with_options(RelativeToolchainBinary-step2
    "-DCMAKE_TOOLCHAIN_FILE=foo.cmake"
  )
endblock()

# Unchanged relative toolchain file in source dir does not trigger reconfigure
block()
  set(RunCMake_TEST_SOURCE_DIR ${RunCMake_BINARY_DIR}/RelativeToolchainSource)
  set(RunCMake_TEST_BINARY_DIR
    ${RunCMake_BINARY_DIR}/RelativeToolchainSource-build
  )
  run_cmake_with_options(RelativeToolchainSource-step1
    "-DCMAKE_TOOLCHAIN_FILE=foo.cmake"
  )
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_with_options(RelativeToolchainSource-step2
    "-DCMAKE_TOOLCHAIN_FILE=foo.cmake"
  )
endblock()

unset(ENV{CMAKE_SYSTEM_ENVIRONMENT_ID})
unset(ENV{CMAKE_SYSTEM_ENVIRONMENT_ACTION})

block()
  set(ENV{CMAKE_SYSTEM_ENVIRONMENT_ID} original)
  run_cmake(SystemEnvironmentId)
  set(RunCMake_TEST_NO_CLEAN 1)

# Unchanged environment id should not emit a warning.
  set(RunCMake_TEST_VARIANT_DESCRIPTION Unchanged)
  run_cmake(SystemEnvironmentId)

  set(ENV{CMAKE_SYSTEM_ENVIRONMENT_ID} different)

# Changed environment id should emit a warning by default.
  set(RunCMake_TEST_VARIANT_DESCRIPTION Changed)
  set(RunCMake-stderr-file SystemEnvironmentIdChanged-stderr.txt)
  run_cmake(SystemEnvironmentId)

# Changed environment id with IGNORE action should not emit a warning.
  set(RunCMake_TEST_VARIANT_DESCRIPTION ChangedIgnore)
  set(ENV{CMAKE_SYSTEM_ENVIRONMENT_ACTION} IGNORE)
  unset(RunCMake-stderr-file)
  run_cmake(SystemEnvironmentId)

# Changed environment id with WARN action should not emit a warning.
  set(RunCMake_TEST_VARIANT_DESCRIPTION ChangedWarn)
  set(ENV{CMAKE_SYSTEM_ENVIRONMENT_ACTION} WARN)
  set(RunCMake-stderr-file SystemEnvironmentIdChanged-stderr.txt)
  run_cmake(SystemEnvironmentId)

# Changed environment id with REFRESH action should refresh the cache.
  set(RunCMake_TEST_VARIANT_DESCRIPTION ChangedRefresh)
  set(ENV{CMAKE_SYSTEM_ENVIRONMENT_ACTION} REFRESH)
  set(RunCMake-stderr-file SystemEnvironmentIdChangedRefresh-stderr.txt)
  run_cmake(SystemEnvironmentId)

# Unrecognized CMAKE_SYSTEM_ENVIRONMENT_ACTION should error.
  set(RunCMake_TEST_VARIANT_DESCRIPTION InvalidAction)
  set(ENV{CMAKE_SYSTEM_ENVIRONMENT_ID} foo)
  set(ENV{CMAKE_SYSTEM_ENVIRONMENT_ACTION} bar)
  set(RunCMake_TEST_EXPECT_RESULT 1)
  set(RunCMake-stderr-file SystemEnvironmentIdInvalidAction-stderr.txt)
  run_cmake(SystemEnvironmentId)
endblock()

unset(ENV{CMAKE_SYSTEM_ENVIRONMENT_ACTION})
unset(ENV{CMAKE_SYSTEM_ENVIRONMENT_ID})

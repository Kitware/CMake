include(RunCMake)

function(run_toolchain_wrappers case)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${case}-build")
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(${case})

  if(RunCMake_GENERATOR MATCHES "Ninja")
    set(verbose_args -- -v)
  endif()
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . ${verbose_args})
endfunction()

set(langs C CXX)
if(CMake_TEST_CUDA)
  list(APPEND langs CUDA)
endif()
if(CMake_TEST_Fortran)
  list(APPEND langs Fortran)
endif()
if(CMake_TEST_HIP)
  list(APPEND langs HIP)
endif()
if(CMake_TEST_ISPC)
  list(APPEND langs ISPC)
endif()
if(CMake_TEST_OBJC)
  list(APPEND langs OBJC OBJCXX)
endif()

foreach(lang ${langs})
  run_toolchain_wrappers(${lang})
endforeach()

run_cmake(DNE)
run_cmake(InterfaceDNE)
run_cmake(DebugProps)
run_toolchain_wrappers(NoWrappers)

# $<COMPILE_LANGUAGE:...>/$<LINK_LANGUAGE:...> generator expressions.
run_toolchain_wrappers(WrapperGenex)
run_toolchain_wrappers(InterfaceGenex)
block()
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/SelectorGenex-build")
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(SelectorGenex)

  if(RunCMake_GENERATOR MATCHES "Ninja")
    set(verbose_args -- -v)
  endif()

  set(RunCMake_TEST_NOT_EXPECT_stdout "SELECTOR")
  run_cmake_command(SelectorGenex-build-mainc
    ${CMAKE_COMMAND} --build . --target mainc ${verbose_args})
  unset(RunCMake_TEST_NOT_EXPECT_stdout)

  run_cmake_command(SelectorGenex-build-maincxx
    ${CMAKE_COMMAND} --build . --target maincxx ${verbose_args})
endblock()

# Compile wrappers of a `$<LINK_ONLY:...>` dependency must not be used.
set(RunCMake_TEST_NOT_EXPECT_stdout "CW_")
run_toolchain_wrappers(LinkOnly)
unset(RunCMake_TEST_NOT_EXPECT_stdout)

run_toolchain_wrappers(SpecialArgs)

# Wrappers are placed outside of launchers.
run_toolchain_wrappers(Launcher)
run_toolchain_wrappers(LauncherEmpty)
run_toolchain_wrappers(LauncherLint)
if(NOT RunCMake_GENERATOR STREQUAL "Watcom WMake")
  run_toolchain_wrappers(CTestLaunch)
endif()

# On most compilers, static libraries are created by an archiver, so link
# wrappers and the linker launcher do not apply. A few toolchains instead
# define CMAKE_<LANG>_CREATE_STATIC_LIBRARY to invoke the compiler driver
# directly, in which case static library creation goes through the normal
# link-command path and link wrappers/launcher *do* apply.
if(NOT CMAKE_C_COMPILER_ID STREQUAL "OrangeC" AND
   NOT (WIN32 AND CMAKE_C_COMPILER_ID STREQUAL "IntelLLVM"))
  set(RunCMake_TEST_NOT_EXPECT_stdout "(LW|LL)_")
endif()
run_toolchain_wrappers(StaticLibrary)
unset(RunCMake_TEST_NOT_EXPECT_stdout)

function(run_toolchain_wrappers_per_config case)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)

  if(RunCMake_GENERATOR MATCHES "Ninja")
    set(verbose_args -- -v)
  endif()

  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${case}-build")
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(${case})

  set(marker_Debug "(CW|LW)_OUTER=1")
  set(marker_Release "(CW|LW)_INNER=2")
  foreach(config IN ITEMS Debug Release)
    # Wrappers for the other config must not leak into this config's commands.
    set(RunCMake_TEST_NOT_EXPECT_stdout "${marker_${config}}")
    run_cmake_command(${case}-${config}-build
      ${CMAKE_COMMAND} --build . --config ${config} ${verbose_args})
  endforeach()
endfunction()
if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  run_toolchain_wrappers_per_config(PerConfig)
endif()

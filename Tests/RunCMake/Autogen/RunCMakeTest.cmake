include(RunCMake)

run_cmake(NoQt)
if (DEFINED with_qt_version)
  set(RunCMake_TEST_OPTIONS
    -Dwith_qt_version=${with_qt_version}
    "-DQt${with_qt_version}_DIR:PATH=${Qt${with_qt_version}_DIR}"
    "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
  )

  run_cmake(QtInFunction)
  run_cmake(QtInFunctionNested)
  run_cmake(QtInFunctionProperty)

  run_cmake(CMP0111-imported-target-full)
  run_cmake(CMP0111-imported-target-libname)
  run_cmake(CMP0111-imported-target-implib-only)

  block()
    set(RunCMake_TEST_BINARY_DIR  ${RunCMake_BINARY_DIR}/MocPredefs-build)
    run_cmake(MocPredefs)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(MocPredefs-build ${CMAKE_COMMAND} --build . --config Debug)
  endblock()

  # Detect information from the toolchain:
  # - CMAKE_INCLUDE_FLAG_CXX
  # - CMAKE_INCLUDE_SYSTEM_FLAG_CXX
  run_cmake(Inspect)
  include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

  if(CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
    if(RunCMake_GENERATOR MATCHES "Visual Studio")
      string(REGEX REPLACE "^-" "/" test_expect_stdout "${CMAKE_INCLUDE_SYSTEM_FLAG_CXX}")
    else()
      set(test_expect_stdout "-*${CMAKE_INCLUDE_SYSTEM_FLAG_CXX}")
    endif()
    string(APPEND test_expect_stdout " *(\"[^\"]*|([^ ]|\\ )*)[\\/]dummy_autogen[\\/]include")
    if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
      string(APPEND test_expect_stdout "_Debug")
    endif()

    block()
      set(RunCMake_TEST_BINARY_DIR  ${RunCMake_BINARY_DIR}/CMP0151-new-build)
      run_cmake_with_options(CMP0151-new ${RunCMake_TEST_OPTIONS} -DCMAKE_POLICY_DEFAULT_CMP0151=NEW)
      set(RunCMake_TEST_NO_CLEAN 1)
      set(RunCMake_TEST_EXPECT_stdout "${test_expect_stdout}")
      message(STATUS "RunCMake_TEST_EXPECT_stdout: ${RunCMake_TEST_EXPECT_stdout}")
      run_cmake_command(CMP0151-new-build ${CMAKE_COMMAND} --build . --config Debug --verbose)
    endblock()

    block()
      set(RunCMake_TEST_BINARY_DIR  ${RunCMake_BINARY_DIR}/AutogenUseSystemIncludeOn-build)
      run_cmake_with_options(AutogenUseSystemIncludeOn ${RunCMake_TEST_OPTIONS} -DCMAKE_POLICY_DEFAULT_CMP0151=NEW)
      set(RunCMake_TEST_NO_CLEAN 1)
      set(RunCMake_TEST_EXPECT_stdout "${test_expect_stdout}")
      message(STATUS "RunCMake_TEST_EXPECT_stdout: ${RunCMake_TEST_EXPECT_stdout}")
      run_cmake_command(AutogenUseSystemIncludeOn ${CMAKE_COMMAND} --build . --config Debug --verbose)
    endblock()
  endif()

  if(CMAKE_INCLUDE_FLAG_CXX)
    if(RunCMake_GENERATOR MATCHES "Visual Studio")
      string(REGEX REPLACE "^-" "/" test_expect_stdout "${CMAKE_INCLUDE_FLAG_CXX}")
    else()
      set(test_expect_stdout "-*${CMAKE_INCLUDE_FLAG_CXX}")
    endif()
    string(APPEND test_expect_stdout " *(\"[^\"]*|([^ ]|\\ )*)[\\/]dummy_autogen[\\/]include")
    if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
      string(APPEND test_expect_stdout "_Debug")
    endif()

    block()
      set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0151-old-build)
      run_cmake_with_options(CMP0151-old ${RunCMake_TEST_OPTIONS} -DCMAKE_POLICY_DEFAULT_CMP0151=OLD)
      set(RunCMake_TEST_NO_CLEAN 1)
      set(RunCMake_TEST_EXPECT_stdout "${test_expect_stdout}")
      message(STATUS "RunCMake_TEST_EXPECT_stdout: ${RunCMake_TEST_EXPECT_stdout}")
      run_cmake_command(CMP0151-old-build ${CMAKE_COMMAND} --build . --config Debug --verbose)
    endblock()

    block()
      set(RunCMake_TEST_BINARY_DIR  ${RunCMake_BINARY_DIR}/AutogenUseSystemIncludeOff-build)
      run_cmake_with_options(AutogenUseSystemIncludeOff ${RunCMake_TEST_OPTIONS} -DCMAKE_POLICY_DEFAULT_CMP0151=NEW)
      set(RunCMake_TEST_NO_CLEAN 1)
      set(RunCMake_TEST_EXPECT_stdout "${test_expect_stdout}")
      message(STATUS "RunCMake_TEST_EXPECT_stdout: ${RunCMake_TEST_EXPECT_stdout}")
      run_cmake_command(AutogenUseSystemIncludeOff ${CMAKE_COMMAND} --build . --config Debug --verbose)
    endblock()

    if(RunCMake_GENERATOR MATCHES "Make|Ninja")
      block()
        set(RunCMake_TEST_BINARY_DIR  ${RunCMake_BINARY_DIR}/AutogenSkipLinting-build)
        list(APPEND RunCMake_TEST_OPTIONS
          "-DPSEUDO_CPPCHECK=${PSEUDO_CPPCHECK}"
          "-DPSEUDO_CPPLINT=${PSEUDO_CPPLINT}"
          "-DPSEUDO_IWYU=${PSEUDO_IWYU}"
          "-DPSEUDO_TIDY=${PSEUDO_TIDY}")

        run_cmake(AutogenSkipLinting)
        set(RunCMake_TEST_NO_CLEAN 1)
        run_cmake_command(AutogenSkipLinting-build ${CMAKE_COMMAND} --build . --config Debug --verbose)
      endblock()
    endif()
  endif()

  if(RunCMake_GENERATOR_IS_MULTI_CONFIG AND NOT RunCMake_GENERATOR MATCHES "Xcode")
    block()
      set(RunCMake_TEST_BINARY_DIR  ${RunCMake_BINARY_DIR}/MocGeneratedFile-build)
      run_cmake(MocGeneratedFile)
      set(RunCMake_TEST_NO_CLEAN 1)
      run_cmake_command(MocGeneratedFile-build ${CMAKE_COMMAND} --build . --config Debug --verbose)
    endblock()
    if(RunCMake_GENERATOR MATCHES "Ninja Multi-Config")
      block()
        set(RunCMake_TEST_BINARY_DIR  ${RunCMake_BINARY_DIR}/MocGeneratedFile-cross-config-build)
        list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_CROSS_CONFIGS=all)
        run_cmake(MocGeneratedFile)
        set(RunCMake_TEST_NO_CLEAN 1)
        run_cmake_command(MocGeneratedFile-cross-config-build ${CMAKE_COMMAND} --build . --config Release --target libgen:Debug)
        run_cmake_command(MocGeneratedFile-cross-config-build ${CMAKE_COMMAND} --build . --config Debug --target libgen:Release)
      endblock()
    endif()
  endif()
endif ()

include(RunCMake)

function(run_TargetMessages case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TargetMessages-${case}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  set(RunCMake_TEST_OPTIONS "${ARGN}")
  run_cmake(TargetMessages-${case})
  run_cmake_command(TargetMessages-${case}-build ${CMAKE_COMMAND} --build .)
endfunction()

run_TargetMessages(ON)
run_TargetMessages(OFF)

run_TargetMessages(VAR-ON -DCMAKE_TARGET_MESSAGES=ON)
run_TargetMessages(VAR-OFF -DCMAKE_TARGET_MESSAGES=OFF)

function(run_VerboseBuild)
  run_cmake(VerboseBuild)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/VerboseBuild-build)
  if(RunCMake_GENERATOR STREQUAL "Watcom WMake")
    # wmake does not actually show the verbose output.
    set(RunCMake-stdout-file VerboseBuild-build-watcom-stdout.txt)
  endif()
  run_cmake_command(VerboseBuild-build ${CMAKE_COMMAND} --build . -v --clean-first)
  unset(RunCMake-stdout-file)
  set(_backup_lang "$ENV{LANG}")
  set(_backup_lc_Messages "$ENV{LC_MESSAGES}")
  if(MAKE_IS_GNU)
    set(RunCMake-stdout-file VerboseBuild-nowork-gnu-stdout.txt)
    set(ENV{LANG} "C")
    set(ENV{LC_MESSAGES} "C")
  endif()
  run_cmake_command(VerboseBuild-nowork ${CMAKE_COMMAND} --build . --verbose)
  set(ENV{LANG} "${_backup_lang}")
  set(ENV{LC_MESSAGES} "${_backup_lc_messages}")
endfunction()
run_VerboseBuild()

function(run_VerboseBuildShort)
  run_cmake(VerboseBuildShort)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/VerboseBuildShort-build)
  if(RunCMake_GENERATOR STREQUAL "Watcom WMake")
    # wmake does not actually show the verbose output.
    set(RunCMake-stdout-file VerboseBuildShort-build-watcom-stdout.txt)
  endif()
  run_cmake_command(VerboseBuildShort-build ${CMAKE_COMMAND} --build . -v --clean-first)
  unset(RunCMake-stdout-file)
  set(_backup_lang "$ENV{LANG}")
  set(_backup_lc_Messages "$ENV{LC_MESSAGES}")
  if(MAKE_IS_GNU)
    set(RunCMake-stdout-file VerboseBuildShort-nowork-gnu-stdout.txt)
    set(ENV{LANG} "C")
    set(ENV{LC_MESSAGES} "C")
  endif()
  run_cmake_command(VerboseBuildShort-nowork ${CMAKE_COMMAND} --build . --verbose)
  set(ENV{LANG} "${_backup_lang}")
  set(ENV{LC_MESSAGES} "${_backup_lc_messages}")
endfunction()
run_VerboseBuildShort()

run_cmake(IncludeRegexSubdir)

function(run_MakefileConflict)
  run_cmake(MakefileConflict)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/MakefileConflict-build)
  run_cmake_command(MakefileConflict-build ${CMAKE_COMMAND} --build . --target Custom)
endfunction()
run_MakefileConflict()

function(run_CMP0113 val)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0113-${val}-build)
  run_cmake(CMP0113-${val})
  set(RunCMake_TEST_NO_CLEAN 1)
  set(_backup_lang "$ENV{LANG}")
  set(_backup_lc_Messages "$ENV{LC_MESSAGES}")
  if(MAKE_IS_GNU)
    set(RunCMake-stderr-file CMP0113-${val}-build-gnu-stderr.txt)
    set(ENV{LANG} "C")
    set(ENV{LC_MESSAGES} "C")
  endif()
  run_cmake_command(CMP0113-${val}-build ${CMAKE_COMMAND} --build .)
  set(ENV{LANG} "${_backup_lang}")
  set(ENV{LC_MESSAGES} "${_backup_lc_messages}")
endfunction()

if(NOT RunCMake_GENERATOR STREQUAL "Watcom WMake")
  run_CMP0113(WARN)
  run_CMP0113(OLD)
  run_CMP0113(NEW)
endif()

function(detect_jobserver_present)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/DetectJobServer-present-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS "-DDETECT_JOBSERVER=${DETECT_JOBSERVER}")
  run_cmake(DetectJobServer-present)
  run_cmake_command(DetectJobServer-present-parallel-build ${CMAKE_COMMAND} --build . -j4)
endfunction()

function(run_make_rule case rule job_count)
  run_cmake_command(${case}-${rule}-j${job_count}
    ${RunCMake_MAKE_PROGRAM} -f "${RunCMake_SOURCE_DIR}/${case}.make" ${rule} -j${job_count}
    CMAKE_COMMAND="${CMAKE_COMMAND}" CMAKE_CTEST_COMMAND="${CMAKE_CTEST_COMMAND}"
    )
endfunction()

function(run_CTestJobServer)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CTestJobServer-build)
  run_cmake(CTestJobServer)
  set(RunCMake_TEST_NO_CLEAN 1)
  # Spoof a number of processors to make sure jobserver integration is unbounded.
  set(ENV{__CTEST_FAKE_PROCESSOR_COUNT_FOR_TESTING} 1)
  run_make_rule(CTestJobServer NoPipe 2)
  run_make_rule(CTestJobServer NoTests 2)
  run_make_rule(CTestJobServer Tests 2)
  run_make_rule(CTestJobServer Tests 3)
  unset(ENV{__CTEST_FAKE_PROCESSOR_COUNT_FOR_TESTING)
endfunction()

# Jobservers are currently only supported by GNU makes, except MSYS2 make
if(MAKE_IS_GNU AND NOT RunCMake_GENERATOR MATCHES "MSYS Makefiles")
  detect_jobserver_present()
  if(UNIX)
    run_CTestJobServer()
  endif()
endif()

if(MAKE_IS_GNU)
  # In GNU makes, `JOB_SERVER_AWARE` support is implemented by prefixing
  # commands with the '+' operator.
  run_cmake(GNUMakeJobServerAware)
endif()

# Output synchronization (-Otarget) is emitted only for the GNU Make family of
# generators.  Drive these cases with a fake "make" so the behavior can be
# asserted independently of the host's real make tool.
if(FAKE_MAKE AND RunCMake_GENERATOR MATCHES "Unix Makefiles|MinGW Makefiles|MSYS Makefiles")
  function(run_OutputSync)
    # Use the fake make for the configure step so the build steps inherit it
    # from the cache.
    set(RunCMake_MAKE_PROGRAM "${FAKE_MAKE}")
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/OutputSync-build)
    set(RunCMake_TEST_NO_CLEAN 1)
    file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
    file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
    run_cmake(OutputSync)

    set(RunCMake-check-file OutputSync-check.cmake)
    set(record "${RunCMake_TEST_BINARY_DIR}/fake_make_record.txt")
    set(marker "${RunCMake_TEST_BINARY_DIR}/fake_make_probe.txt")
    set(ENV{FAKE_MAKE_RECORD} "${record}")
    set(ENV{FAKE_MAKE_PROBE_MARKER} "${marker}")

    # CMake-driven parallel build with GNU Make >= 4.0: group the output.
    file(REMOVE "${record}" "${marker}")
    set(ENV{FAKE_MAKE_VERSION} "GNU Make 4.4.1")
    set(expect_otarget 1)
    set(expect_uses_terminal_flag 1)
    set(expect_probe 1)
    run_cmake_command(OutputSync-parallel ${CMAKE_COMMAND} --build . --parallel 2)

    # Serial build: no grouping and no probe.
    file(REMOVE "${record}" "${marker}")
    set(expect_otarget 0)
    set(expect_uses_terminal_flag 0)
    set(expect_probe 0)
    run_cmake_command(OutputSync-serial ${CMAKE_COMMAND} --build .)

    # Native "-- -j" opt-out: CMake adds neither -j nor -Otarget, no probe.
    file(REMOVE "${record}" "${marker}")
    set(expect_otarget 0)
    set(expect_uses_terminal_flag 0)
    set(expect_probe 0)
    run_cmake_command(OutputSync-native-j ${CMAKE_COMMAND} --build . -- -j)

    # A user's native "-O" overrides CMake's: -Otarget appears before -Onone.
    file(REMOVE "${record}" "${marker}")
    set(expect_otarget 1)
    set(expect_uses_terminal_flag 1)
    set(expect_probe 1)
    set(expect_order_onone 1)
    run_cmake_command(OutputSync-override ${CMAKE_COMMAND} --build . --parallel 2 -- -Onone)
    unset(expect_order_onone)

    # GNU Make < 4.0 does not support --output-sync.
    file(REMOVE "${record}" "${marker}")
    set(ENV{FAKE_MAKE_VERSION} "GNU Make 3.81")
    set(expect_otarget 0)
    set(expect_uses_terminal_flag 0)
    set(expect_probe 1)
    run_cmake_command(OutputSync-old ${CMAKE_COMMAND} --build . --parallel 2)

    # 4.0 is the first release to support --output-sync; verify the lower
    # boundary and the rest of the 4.0-4.2 series are classified as supported.
    foreach(v IN ITEMS 4.0 4.1 4.2)
      file(REMOVE "${record}" "${marker}")
      set(ENV{FAKE_MAKE_VERSION} "GNU Make ${v}")
      set(expect_otarget 1)
      set(expect_uses_terminal_flag 1)
      set(expect_probe 1)
      run_cmake_command(OutputSync-v${v} ${CMAKE_COMMAND} --build . --parallel 2)
    endforeach()

    # A non-GNU make is not grouped.
    file(REMOVE "${record}" "${marker}")
    set(ENV{FAKE_MAKE_VERSION} "bmake version 20200710")
    set(expect_otarget 0)
    set(expect_uses_terminal_flag 0)
    set(expect_probe 1)
    run_cmake_command(OutputSync-nongnu ${CMAKE_COMMAND} --build . --parallel 2)

    # Unparsable "--version" output: probe fails gracefully, build still runs.
    file(REMOVE "${record}" "${marker}")
    set(ENV{FAKE_MAKE_VERSION} "garbage output")
    set(expect_otarget 0)
    set(expect_uses_terminal_flag 0)
    set(expect_probe 1)
    run_cmake_command(OutputSync-garbage ${CMAKE_COMMAND} --build . --parallel 2)

    # Probe exits non-zero: treated as unsupported, build still runs.
    file(REMOVE "${record}" "${marker}")
    set(ENV{FAKE_MAKE_VERSION} "GNU Make 4.4.1")
    set(ENV{FAKE_MAKE_VERSION_RESULT} "2")
    set(expect_otarget 0)
    set(expect_uses_terminal_flag 0)
    set(expect_probe 1)
    run_cmake_command(OutputSync-probe-fail ${CMAKE_COMMAND} --build . --parallel 2)
    unset(ENV{FAKE_MAKE_VERSION_RESULT})

    # --clean-first runs two build commands but probes only once (memoized).
    file(REMOVE "${record}" "${marker}")
    set(ENV{FAKE_MAKE_VERSION} "GNU Make 4.4.1")
    set(expect_otarget 1)
    set(expect_otarget_count 2)
    set(expect_uses_terminal_flag 1)
    set(expect_probe 1)
    set(expect_probe_count 1)
    run_cmake_command(OutputSync-clean-first ${CMAKE_COMMAND} --build . --parallel 2 --clean-first)
    unset(expect_otarget_count)
    unset(expect_probe_count)

    unset(ENV{FAKE_MAKE_VERSION})
    unset(ENV{FAKE_MAKE_RECORD})
    unset(ENV{FAKE_MAKE_PROBE_MARKER})
  endfunction()
  run_OutputSync()

  # USES_TERMINAL recipe prefixing is a generation-time change; inspect the
  # generated build.make directly.
  function(run_OutputSyncUsesTerminal)
    set(RunCMake_MAKE_PROGRAM "${FAKE_MAKE}")
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/OutputSyncUsesTerminal-build)
    set(RunCMake_TEST_NO_CLEAN 1)
    file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
    file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
    run_cmake(OutputSyncUsesTerminal)
  endfunction()
  run_OutputSyncUsesTerminal()
endif()

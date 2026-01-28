set(CTEST_MEMORYCHECK_TYPE "$ENV{CTEST_MEMORYCHECK_TYPE}")
set(CTEST_MEMORYCHECK_SANITIZER_OPTIONS "$ENV{CTEST_MEMORYCHECK_SANITIZER_OPTIONS}")

set(lsan_suppressions "${CMAKE_CURRENT_LIST_DIR}/ctest_memcheck_$ENV{CMAKE_CONFIGURATION}.lsan.supp")
if (EXISTS "${lsan_suppressions}")
  set(ENV{LSAN_OPTIONS} "suppressions='${lsan_suppressions}'")
endif ()

if (CTEST_MEMORYCHECK_TYPE STREQUAL "Valgrind")
  find_program(valgrind_exe NAMES valgrind)
  set(CTEST_MEMORYCHECK_COMMAND "${valgrind_exe}")

  set(valgrind_suppressions "${CMAKE_CURRENT_LIST_DIR}/ctest_memcheck_$ENV{CMAKE_CONFIGURATION}.valgrind.supp")
  set(common_valgrind_suppressions "${CMAKE_CURRENT_LIST_DIR}/ctest_memcheck_$ENV{CMAKE_VALGRIND_CONFIGURATION}.valgrind.supp")
  if (EXISTS "${valgrind_suppressions}")
    set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE "${valgrind_suppressions}")
  elseif (EXISTS "${common_valgrind_suppressions}")
    set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE "${common_valgrind_suppressions}")
  endif ()

  set(valgrind_skip
    /bin/*
    /sbin/*
    /usr/bin/*
    /usr/lib64/qt5/bin/*
    /usr/lib64/qt6/bin/*
    /usr/lib64/qt6/libexec/*
    bootstrap
    sample_script
    */Tests/CTestTest2/kwsysBin/*
    */Tests/CTestTestCrash/Crash
    */Tests/Qt*Autogen/*
    # Ignore ISPC files which may contain unimplemented instructions.
    */build/Tests/ISPC/TryCompile/ISPCTryCompile
    # Ignore anything CI downloads.
    */.gitlab/*)
  list(JOIN valgrind_skip "," valgrind_skip)
  string(CONCAT valgrind_options
    "--gen-suppressions=all "
    "--child-silent-after-fork=yes "
    "--trace-children=yes "
    "--trace-children-skip=${valgrind_skip} "
    "--track-origins=yes "
    "-q "
    "--leak-check=yes "
    "--show-reachable=yes "
    "--num-callers=50 "
    "-v ")
  set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "${valgrind_options}")
endif ()

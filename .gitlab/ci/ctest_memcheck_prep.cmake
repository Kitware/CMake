set(CTEST_MEMORYCHECK_TYPE "$ENV{CTEST_MEMORYCHECK_TYPE}")
set(CTEST_MEMORYCHECK_SANITIZER_OPTIONS "$ENV{CTEST_MEMORYCHECK_SANITIZER_OPTIONS}")

set(lsan_suppressions "${CMAKE_CURRENT_LIST_DIR}/ctest_memcheck_$ENV{CMAKE_CONFIGURATION}.lsan.supp")
if (EXISTS "${lsan_suppressions}")
  set(ENV{LSAN_OPTIONS} "suppressions='${lsan_suppressions}'")
endif ()

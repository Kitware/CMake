ctest_memcheck-leak_sanitizer
=============================

* The :command:`ctest_memcheck` command learned to support ``LeakSanitizer``
  independently from ``AddressSanitizer``.

* The :command:`ctest_memcheck` command learned to read the location of
  suppressions files for sanitizers from the
  :variable:`CTEST_MEMORYCHECK_SUPPRESSIONS_FILE` variable.

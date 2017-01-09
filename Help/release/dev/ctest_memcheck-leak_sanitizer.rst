ctest_memcheck-leak_sanitizer
=============================

* The :command:`ctest_memcheck` command learned to support ``LeakSanitizer``
  independently from ``AddressSanitizer``.

* The :command:`ctest_memcheck` command learned to read the location of
  suppressions files for sanitizers from the
  :variable:`CTEST_MEMORYCHECK_SUPPRESSIONS_FILE` variable.

* The :command:`ctest_memcheck` command was fixed to correctly append extra
  sanitizer options read from the
  :variable:`CTEST_MEMORYCHECK_SANITIZER_OPTIONS` variable to the environment
  variables used internally by the sanitizers.

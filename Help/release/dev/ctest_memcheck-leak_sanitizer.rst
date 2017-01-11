ctest_memcheck-leak_sanitizer
=============================

* The :command:`ctest_memcheck` command learned to support ``LeakSanitizer``
  independently from ``AddressSanitizer``.

* The :command:`ctest_memcheck` command no longer automatically adds
  ``leak_check=1`` to the options used by ``AddressSanitizer``. The default
  behavior of ``AddressSanitizer`` is to run `LeakSanitizer` to check leaks
  unless ``leak_check=0``.

* The :command:`ctest_memcheck` command learned to read the location of
  suppressions files for sanitizers from the
  :variable:`CTEST_MEMORYCHECK_SUPPRESSIONS_FILE` variable.

* The :command:`ctest_memcheck` command was fixed to correctly append extra
  sanitizer options read from the
  :variable:`CTEST_MEMORYCHECK_SANITIZER_OPTIONS` variable to the environment
  variables used internally by the sanitizers.

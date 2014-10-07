ctest-memcheck-sanitizers
-------------------------

* The :command:`ctest_memcheck` command learned to support sanitizer
  modes, including ``AddressSanitizer``, ``MemorySanitizer``,
  ``ThreadSanitizer``, and ``UndefinedBehaviorSanitizer``.
  Options may be set using the new
  :variable:`CTEST_MEMORYCHECK_SANITIZER_OPTIONS` variable.

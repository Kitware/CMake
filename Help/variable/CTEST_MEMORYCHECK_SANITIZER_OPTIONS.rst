CTEST_MEMORYCHECK_SANITIZER_OPTIONS
-----------------------------------

.. versionadded:: 3.1

Specify the CTest ``MemoryCheckSanitizerOptions`` setting
in a :manual:`ctest(1)` :ref:`Dashboard Client` script,
or on the :program:`ctest` command line via the :ref:`-D <ctest-option-D-var>` option.

CTest prepends correct sanitizer options ``*_OPTIONS``
environment variable to executed command. CTests adds
its own ``log_path`` to sanitizer options, don't provide your
own ``log_path``.

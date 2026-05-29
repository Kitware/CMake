CTEST_MEMORYCHECK_TYPE
----------------------

.. versionadded:: 3.1

Specify the CTest ``MemoryCheckType`` setting
in a :manual:`ctest(1)` :ref:`Dashboard Client` script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D` option.
Valid values are ``Valgrind``, ``Purify``, ``BoundsChecker``, ``DrMemory``,
``CudaSanitizer``, ``ThreadSanitizer``, ``AddressSanitizer``, ``LeakSanitizer``,
``MemorySanitizer`` and ``UndefinedBehaviorSanitizer``.

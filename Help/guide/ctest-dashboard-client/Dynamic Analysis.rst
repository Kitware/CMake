.. _`CTest Dashboard Step 4`:

Step 4: Dynamic Analysis
========================

This step shows how to detect memory errors with two tools:
AddressSanitizer and Valgrind.

Unlike the other steps, this one builds its own project in the
``Step4/Project`` directory.  Its source file writes to memory after it is
freed:

.. literalinclude:: Step4/Project/use_after_free.c
  :caption: Step4/Project/use_after_free.c
  :language: c

Both scripts call :command:`ctest_memcheck` instead of :command:`ctest_test`.
This runs each test under the analysis tool, collects the problems it
reports, and submits them to CDash separately from the test results.

AddressSanitizer
^^^^^^^^^^^^^^^^

AddressSanitizer is built into GCC and Clang.  The project is compiled with
``-fsanitize=address``, and the tests report memory errors when they run.

.. literalinclude:: Step4/asan.cmake
  :caption: Step4/asan.cmake
  :language: cmake

Setting :variable:`CTEST_MEMORYCHECK_TYPE` tells CTest how to parse the
sanitizer output, so that it can report each problem to CDash.

.. code-block:: shell

  ctest -S Step4/asan.cmake -V

Valgrind
^^^^^^^^

Valgrind runs each test executable under its own instrumentation, so it
needs no special compiler flags.  It is more portable but slower than ASan.

.. literalinclude:: Step4/valgrind.cmake
  :caption: Step4/valgrind.cmake
  :language: cmake

.. code-block:: shell

  ctest -S Step4/valgrind.cmake -V

Requirements
^^^^^^^^^^^^

* AddressSanitizer: GCC or Clang with ``-fsanitize=address``.
* Valgrind: ``valgrind`` found in the ``PATH``.

See the :ref:`CTest MemCheck Step` for other supported tools.

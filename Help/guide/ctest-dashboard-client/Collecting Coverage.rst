.. _`CTest Dashboard Step 3`:

Step 3: Collecting Coverage
===========================

This step extends :ref:`Step 2 <CTest Dashboard Step 2>` by collecting code
coverage and submitting it to CDash.  It does not enable the build error
from Step 2.

.. literalinclude:: Step3/submit.cmake
  :caption: Step3/submit.cmake
  :language: cmake

Two things are needed to collect coverage:

1. Build the project with instrumentation.  The ``--coverage`` flag makes
   GCC and Clang add counters that record which lines run.  A ``Debug``
   build avoids optimizations that would make the line counts less
   accurate.

2. Run the coverage tool.  After :command:`ctest_test`, the
   :command:`ctest_coverage` command runs the tool given by
   :variable:`CTEST_COVERAGE_COMMAND` on the data the tests produced.
   CTest parses its output and submits the coverage of each file.

On CDash, the build shows the overall coverage percentage.  The details
show the coverage of each file, with the lines that ran and the lines that
did not.  For example, the ``fails`` test never runs one line of
``fails.cxx``:

.. literalinclude:: Project/fails.cxx
  :caption: Project/fails.cxx
  :language: c++

Requirements
^^^^^^^^^^^^

* GCC or Clang (supports ``--coverage`` / ``-fprofile-arcs -ftest-coverage``)
* A ``gcov`` tool that matches the compiler version, found in the ``PATH``.

See the :ref:`CTest Coverage Step` for other supported tools.

Running the Script
^^^^^^^^^^^^^^^^^^

.. code-block:: shell

  ctest -S Step3/submit.cmake -V

.. _`CTest Dashboard Step 5`:

Step 5: Instrumentation
=======================

.. versionadded:: 4.3

This step extends :ref:`Step 3 <CTest Dashboard Step 3>` by enabling
:manual:`instrumentation <cmake-instrumentation(7)>`, which records the
time taken by each configure, build, and test command and submits it to
CDash.

.. literalinclude:: Step5/submit.cmake
  :caption: Step5/submit.cmake
  :language: cmake
  :end-at: CTEST_USE_INSTRUMENTATION

Setting the :envvar:`CTEST_USE_INSTRUMENTATION` environment variable before
the dashboard steps run enables instrumentation.  The rest of the script is
the same as Step 3.

The variable can also be set by the environment that runs the script, such
as a CI job definition.  This lets the same script run with or without
instrumentation:

.. code-block:: shell

  CTEST_USE_INSTRUMENTATION=ON ctest -S Step5/submit.cmake -V

See the :manual:`cmake-instrumentation(7)` manual for details about the
data that is submitted.

Requirements
^^^^^^^^^^^^

* The same as :ref:`Step 3 <CTest Dashboard Step 3>`.

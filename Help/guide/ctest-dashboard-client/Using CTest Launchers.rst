.. _`CTest Dashboard Step 2`:

Step 2: Using CTest Launchers
=============================

This step extends :ref:`Step 1 <CTest Dashboard Step 1>` by enabling CTest
launchers, which give more detailed reports of build warnings and errors.

.. literalinclude:: Step2/submit.cmake
  :caption: Step2/submit.cmake
  :language: cmake

When :variable:`CTEST_USE_LAUNCHERS` is enabled, CTest wraps each compile,
link, and custom command in a launcher.  If a command produces a warning or
error, CTest records its command line, working directory, and output, and
includes them in the submission.  Without launchers, CTest only scans the
whole build output, which makes it harder to tell which command produced a
given message.

To show this, the script configures the project with the
``DASHBOARD_EXAMPLE_BUILD_ERROR`` option, which adds a target whose source
file calls an undeclared function:

.. literalinclude:: Project/error.cxx
  :caption: Project/error.cxx
  :language: c++

On CDash, the build shows the compiler command line that failed, the error
message, and the source file and line.  This makes triage significantly faster
on large projects where a single failing compilation step may be buried in
megabytes of build output.

Depending on the build tool, the build may stop at the error before it
builds the test executables.  In that case, the ``Test`` step reports that
it cannot find them.

Launchers require that the project includes the :module:`CTest` module, and
that the project is configured by :command:`ctest_configure` so that CTest
can add the launchers to the build system.

Requirements
^^^^^^^^^^^^

* A build tool supporting launchers (the :ref:`Makefile Generators` or
  :ref:`Ninja Generators`).

Running the Script
^^^^^^^^^^^^^^^^^^

.. code-block:: shell

  ctest -S Step2/submit.cmake -V

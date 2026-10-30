.. _`CTest Dashboard Step 9`:

Step 9: Using the Command Line
==============================

.. versionadded:: 4.4

This step extends :ref:`Step 8 <CTest Dashboard Step 8>` by replacing the
dashboard script with :program:`ctest` command lines in a shell script.

.. literalinclude:: Step9/submit.sh
  :caption: Step9/submit.sh
  :language: shell

Each :program:`ctest` invocation runs the steps given by
:option:`ctest -T`, and sets variables with
:option:`-D \<var\>=\<value\> <ctest-dashboard -D>` instead of the
:command:`set` command.

The first invocation configures an empty build tree, using the
:ctest-dashboard-option:`--source-dir` and
:ctest-dashboard-option:`--build-dir` options.  The later invocations run in
that build tree and add their results to the same submission.

Each invocation ends with a ``Submit`` step.  The
:variable:`CTEST_SUBMIT_PARTS` variable limits it to the results of that
invocation, so that earlier results are not submitted again.  The last
invocation also submits the ``Done`` part, which tells CDash that the build
is complete.

A dashboard script and command lines each have advantages:

* A script can use the CMake language for logic, such as loops and
  conditions, and keeps every step in one file.
* Command lines can run the steps separately, such as in different jobs of
  a CI pipeline, and need no ``.cmake`` file.

Requirements
^^^^^^^^^^^^

* GCC or Clang, which support the ``--coverage`` flag.
* A ``gcov`` tool that matches the compiler version, found in the ``PATH``.
* A POSIX shell to run the script.

Running the Script
^^^^^^^^^^^^^^^^^^

.. code-block:: shell

  sh Step9/submit.sh

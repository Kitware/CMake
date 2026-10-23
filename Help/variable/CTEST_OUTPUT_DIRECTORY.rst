CTEST_OUTPUT_DIRECTORY
----------------------

.. versionadded:: 4.6

Specify the directory in which :manual:`ctest(1)` writes its output files
in a :ref:`Dashboard Client` script.

CTest writes the files it produces during a run, such as the ``TAG`` file,
the logs under ``Temporary``, and the XML files submitted to a dashboard,
into a single output directory.  By default this is the ``Testing``
directory under :variable:`CTEST_BINARY_DIRECTORY`.  Set this variable
before calling :command:`ctest_start` to write them elsewhere; the path may
be outside the build tree, and a relative path is interpreted with respect
to :variable:`CTEST_BINARY_DIRECTORY`.

This makes it possible to run more than one :program:`ctest` process
against the same build tree, for example to run different subsets of the
tests with the ``INCLUDE_FROM_FILE`` option of :command:`ctest_test`,
without the processes overwriting each other's output.

Outside of a dashboard script, use the :option:`ctest --output-dir` option.

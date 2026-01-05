ctest-output-dir
----------------

* :manual:`ctest(1)` gained an :option:`--output-dir <ctest --output-dir>`
  option, and the corresponding :variable:`CTEST_OUTPUT_DIRECTORY` variable
  for :ref:`Dashboard Client` scripts, to specify the directory in which
  CTest writes its output files.  Previously these were always written to
  the ``Testing`` directory under the build tree, which prevented running
  more than one :program:`ctest` process against the same build tree.

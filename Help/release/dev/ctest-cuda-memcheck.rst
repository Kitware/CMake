CTest
-----

* :manual:`ctest(1)` gained support for cuda-memcheck as ``CTEST_MEMORYCHECK_COMMAND``.
  The different tools (memcheck, racecheck, synccheck, initcheck) supplied by
  cuda-memcheck can be selected by setting the appropriate flags using the
  ``CTEST_MEMORYCHECK_COMMAND_OPTIONS`` variable.
  The default flags are `--tool memcheck --leak-check full`.

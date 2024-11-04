ctest-crash-handling
--------------------

* The :option:`ctest --interactive-debug-mode` option on Windows
  now enables Windows Error Reporting by default in test processes,
  allowing them to creating debug popup windows and core dumps.
  This restores behavior previously removed by CMake 3.11.

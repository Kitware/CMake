labels_for_subprojects
----------------------

* A :variable:`CTEST_LABELS_FOR_SUBPROJECTS` CTest module variable and CTest
  script variable was added to specify a list of labels that should be treated
  as subprojects by CDash. To use this value in both the CTest module and the
  ctest command line `Dashboard Client` mode (e.g. ctest -S) set it in the
  CTestConfig.cmake config file.

* A :prop_dir:`LABELS` directory property was added to specify labels
  for all targets and tests in a directory.

* A :variable:`CMAKE_DIRECTORY_LABELS` variable was added to specify
  labels for all tests in a directory.

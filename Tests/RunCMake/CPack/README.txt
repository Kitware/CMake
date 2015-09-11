RunCMake.CPack is a test module that is intended for testing of package
generators that can be validated from command line.

-------------
Adding a test
-------------

CPack test root directory: 'Tests/RunCMake/CPack'.

All phases are executed separately for each generator that is bound to a test.
Tests for each generator are subtests of test 'RunCMake.CPack_<generator_name>'.

Each test must also be added to 'RunCMakeTest.cmake' script located in CPack
test root directory.
Line that adds a test is:
run_cpack_test(<test_name> "<generator_name>")

<generator_name> may be one generator e.g. "RPM" or multiple e.g. "RPM;DEB" and
will be run for all listed generators.

Test consists of
- CMake execution phase
- CPack execution phase
- verification of generated files

CMake execution phase:
----------------------

To add a new CPack test we first create a <test_name>.cmake script that contains
CMake commands that should be used as a preparation script for generation of
different types of packages. This script is placed into CPack test root
directory even if it will be used for only one of the generators.

If test will be used for multiple generators but some of them require some
generator specific commands then those commands should be added to a separate
file that should be located in '<generator_name>/<test_name>-specifics.cmake'
in CPack test root directory.

CPack execution phase:
----------------------

Only executes CPack for content that was generated during CMake execution
phase.

Verification of generated files:
--------------------------------

Verification of generated files consists of two phases
- mandatory verification phase
- optional verification phase

Mandatory verification phase checks that expected files were generated and
contain expected files.
Mandatory verification phase also checks that no other unexpected package files
were generated (this is executed even if EXPECTED_FILES_COUNT contains 0 in
order to verify that no files were generated).
CMake script '<generator_name>/<test_name>-ExpectedFiles.cmake' is required by
this step and must contain
- EXPECTED_FILES_COUNT variable that contains the number of expected files that
  will be generated (0 or more)
- EXPECTED_FILE_<file_number_starting_with_1> that contains globing expression
  that uniquely defines expected file name (will be used to find expected file)
  and should be present once for each expected file
- EXPECTED_FILE_CONTENT_<file_number_starting_with_1> that contains regular
  expression of files that should be present in generated file and should be
  present once for each expected file

Optional verification phase is generator specific and is optionaly executed.
This phase is executed if '<generator_name>/<test_name>-VerifyResult.cmake'
script exists.
In case that the script doesn't exist VerifyResult.cmake script automatically
prints out standard output and standard error from CPack execution phase that
is compared with '<generator_name>/<test_name>-stdout.txt' regular expression
and '<generator_name>/<test_name>-stderr.txt' regular expresson respectively.

----------------------
Adding a new generator
----------------------

To add a new generator we must
- add new generator directory (e.g. RPM for RPM generator) to CPack test root
  directory that contains 'Helpers.cmake' script. In this script a function
  named 'getPackageContent' must exist. This function should list files that
  are contained in a package. Function should accept two parameters
  + FILE variable that will contain path to file for which the content should be
    listed
  + RESULT_VAR that will tell the function which variable in parent scope should
    contain the result (list of files inside package file)
- add 'Prerequirements.cmake' script to generator directory. In this script a
  function named 'get_test_prerequirements' must exist. This function should
  set a variable in parent scope (name of the variable is the first parameter)
  that tells if prerequirements for test execution are met (certain programs,
  OS specifics, ...) and create a config file (name of the variable which
  contains file name and path is provided with the second parameter) that
  should contain 'set' commands for variables that will later be used in tests
  (e.g. location of dpkg program for DEB packages)
- add tests the same way as described above
- add generator to 'add_RunCMake_test_group' function call that is located in
  RunCMake CMakeLists.txt file

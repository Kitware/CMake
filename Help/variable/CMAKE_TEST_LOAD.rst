CMAKE_TEST_LOAD
------------------

CMake variable to specify the CTest ``TestLoad`` setting.
See :variable:`CTEST_TEST_LOAD` for more details.

Note that this variable is not honored during script mode.
If you intend to drive your tests with ``ctest -S``, please set
:variable:`CTEST_TEST_LOAD` in your dashboard script instead.

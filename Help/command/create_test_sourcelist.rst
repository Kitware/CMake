create_test_sourcelist
----------------------

Create a test driver program that links together many small tests into a
single executable.  This is useful when building static executables with
large libraries to shrink the total required size.

.. signature::
  create_test_sourcelist(<sourceListName> <driverName> <test>... <options>...)
  :target: original

  Generate a test driver source file from a list of individual test sources
  and provide a combined list of sources that can be built as an executable.

  The options are:

  ``<sourceListName>``
    The name of a variable in which to store the list of source files needed
    to build the test driver.  The list will contain the ``<test>...`` sources
    and the generated ``<driverName>`` source.

    .. versionchanged:: 3.29

      The test driver source is listed by absolute path in the build tree.
      Previously it was listed only as ``<driverName>``.

  ``<driverName>``
    Name of the test driver source file to be generated into the build tree.
    The source file will contain a ``main()`` program entry point that
    dispatches to whatever test is named on the command line.

  ``<test>...``
    Test source files to be added to the driver binary.  Each test source
    file must have a function in it that is the same name as the file with the
    extension removed.  For example, a ``foo.cxx`` test source might contain:

    .. code-block:: c++

      int foo(int argc, char** argv)

  ``EXTRA_INCLUDE <header>``
    Specify a header file to ``#include`` in the generated test driver source.

  ``FUNCTION <function>``
    Specify a function to be called with pointers to ``argc`` and ``argv``.
    The function may be provided in the ``EXTRA_INCLUDE`` header:

    .. code-block:: c++

      void function(int* pargc, char*** pargv)

    This can be used to add extra command line processing to each test.

Additionally, some CMake variables affect test driver generation:

.. variable:: CMAKE_TESTDRIVER_BEFORE_TESTMAIN

  Code to be placed directly before calling each test's function.

.. variable:: CMAKE_TESTDRIVER_AFTER_TESTMAIN

  Code to be placed directly after the call to each test's function.

The generated test driver supports the following command-line arguments:

``<name>``
  Run the test with the exact name ``<name>`` (case-insensitive).

``-R <substr>``
  Run the first test whose name contains ``<substr>`` (case-insensitive).

``-A [<skip_test>...]``
  .. versionadded:: 3.21

    Run all tests and print results in `TAP <https://testanything.org/>`_
    format.

    Any additional arguments after ``-A`` are interpreted as exact test names
    to skip.

``-N``
  .. versionadded:: 4.4

    List all available test names (one per line) and exit.

Example
^^^^^^^

.. code-block:: cmake

  create_test_sourcelist(SRCS main.c test1.c test2.c)
  add_executable(MyTests ${SRCS})
  discover_tests(COMMAND MyTests
    DISCOVERY_ARGS -N
    DISCOVERY_MATCH "^(.+)$"
    TEST_NAME "${PROJECT_NAME}.\\1"
    TEST_ARGS "\\1"
  )

See Also
^^^^^^^^

* :command:`discover_tests`

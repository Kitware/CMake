discover_tests
--------------

.. versionadded:: 4.4

Register tests with names and properties discovered at test time by
:manual:`ctest(1)`.

.. code-block:: cmake

  discover_tests(COMMAND <command> [<arg>...] [COMMAND_EXPAND_LISTS]
    [CONFIGURATIONS <config>...]
    DISCOVERY_ARGS <arg>...
    DISCOVERY_MATCH <regex>
    [DISCOVERY_PROPERTIES <key> <value> [<key> <value>]...]
    TEST_NAME <replacement>
    TEST_ARGS <replacement>...
    [TEST_PROPERTIES <key> <replacement> [<key> <replacement>]...]
    [BUILD_DEPENDS <dependencies>...]
  )

This command configures test discovery rather than defining a single test at
configure time.  During test execution, :manual:`ctest(1)` runs the specified
discovery command, parses its output, and registers one or more tests based on
the provided regular expression and replacement strings.

``discover_tests`` options are:

``COMMAND``
  Specify the command-line used for test discovery.

  The command is executed by :manual:`ctest(1)` at test time (not by CMake at
  configure time).  With ``DISCOVERY_ARGS`` appended, it must print the list of
  available tests in a format matched by ``DISCOVERY_MATCH``.

  If ``<command>`` specifies an executable target created by
  :command:`add_executable`:

  * It will automatically be replaced by the location of the executable
    created at build time.

  * The target's :prop_tgt:`CROSSCOMPILING_EMULATOR`, if set, will be
    used to run the command on the host::

      <emulator> <command>

    The emulator is used only when
    :variable:`cross-compiling <CMAKE_CROSSCOMPILING>`.

  * The target's :prop_tgt:`TEST_LAUNCHER`, if set, will be used to launch the
    command::

      <launcher> <command>

    If the :prop_tgt:`CROSSCOMPILING_EMULATOR` is also set, both are used::

      <launcher> <emulator> <command>

  The command may be specified using
  :manual:`generator expressions <cmake-generator-expressions(7)>`.

``COMMAND_EXPAND_LISTS``
  Lists in ``COMMAND`` arguments will be expanded, including those created with
  :manual:`generator expressions <cmake-generator-expressions(7)>`.

``CONFIGURATIONS``
  Restrict the test discovery only to the named configurations.

``DISCOVERY_ARGS``
  Additional arguments passed to ``COMMAND`` when performing discovery.

``DISCOVERY_MATCH``
  Regular expression used to parse each line produced by the discovery command.
  Capturing groups may be referenced by ``TEST_NAME``, ``TEST_ARGS``, and
  values in ``TEST_PROPERTIES`` using ``\1``, ``\2``, etc.

``DISCOVERY_PROPERTIES``
  Specify properties for the discovery run itself.

``TEST_NAME``
  Replacement string used to generate the test name for each discovered test.
  It may reference capture groups from ``DISCOVERY_MATCH``.

``TEST_ARGS``
  Replacement strings used to generate the arguments passed to the discovered
  test.  Each argument may reference capture groups from ``DISCOVERY_MATCH``.

``TEST_PROPERTIES``
  Specify test properties to set on each discovered test.  Values are
  replacement strings and may reference capture groups from
  ``DISCOVERY_MATCH``.

  .. versionchanged:: 4.5
    If the properties make the discovered tests part of a test fixture, and
    they do not set :prop_test:`FIXTURE_REPEAT_MODE`, policy :policy:`CMP0224`
    determines the behavior when the :ctest-option:`--repeat` option is used.

``BUILD_DEPENDS``
  Specify a list of targets or files that must be built before the test
  discovery can run.

  When the :variable:`CMAKE_TEST_BUILD_DEPENDS` variable is enabled with the
  :ref:`Ninja Generators`, :generator:`FASTBuild`, or :ref:`Makefile
  Generators`, these dependencies are built by the ``test_prep/all`` target.
  If ``COMMAND`` names an executable target created by
  :command:`add_executable`, that target is also added automatically.

  Individual ``test_prep/<test-name>`` targets are not created for discovered
  tests because their names are not known at generation time.

  Build dependencies added by this argument, and an executable target
  named by ``COMMAND``, also enable the
  :ctest-option:`--out-of-date` behavior of :manual:`ctest(1)`.

  Tests with missing build dependencies will not be run.

CTest executes the discovery step to obtain the list of tests and then runs
each discovered test using the command-line produced by ``COMMAND`` together
with ``TEST_ARGS``.  The pass/fail behavior of each discovered test follows
the usual CTest rules (exit code ``0`` indicates success unless inverted by
the :prop_test:`WILL_FAIL` property). Output written to stdout or stderr is
captured by :manual:`ctest(1)` and only affects the pass/fail status via the
:prop_test:`PASS_REGULAR_EXPRESSION`, :prop_test:`FAIL_REGULAR_EXPRESSION`,
or :prop_test:`SKIP_REGULAR_EXPRESSION` test properties.

Example usage:

.. code-block:: cmake

  discover_tests(COMMAND testDriver --exe $<TARGET_FILE:myexe>
    DISCOVERY_ARGS --list-tests
    DISCOVERY_MATCH "^([^,]+),([^,]+),([^,]+),(.*)$"
    TEST_NAME "${PROJECT_NAME}.\\1.\\2"
    TEST_ARGS --run-test "\\1.\\2"
    TEST_PROPERTIES
      PROCESSORS "\\3"
      LABELS "\\4"
  )

This example configures discovery by running ``testDriver --list-tests``.
For each line of output that matches ``DISCOVERY_MATCH``, a test name is
generated using ``TEST_NAME``, the per-test command-line is generated using
``TEST_ARGS``, and test properties are populated from the remaining capture
groups.

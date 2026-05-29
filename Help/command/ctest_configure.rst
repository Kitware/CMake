ctest_configure
---------------

Perform the :ref:`CTest Configure Step` as a :ref:`Dashboard Client`.

.. code-block:: cmake

  ctest_configure([BUILD <build-dir>] [SOURCE <source-dir>] [APPEND]
                  [OPTIONS <options>] [RETURN_VALUE <result-var>] [QUIET]
                  [CAPTURE_CMAKE_ERROR <result-var>]
                  [PRESET <preset>]
                  [PRESETS_FILE <file>]
                  )

Configure the project build tree and record results in ``Configure.xml``
for submission with the :command:`ctest_submit` command.

The options are:

``BUILD <build-dir>``
  Specify the top-level build directory.  If not given, the
  :variable:`CTEST_BINARY_DIRECTORY` variable is used.

``SOURCE <source-dir>``
  Specify the source directory.  If not given, the
  :variable:`CTEST_SOURCE_DIRECTORY` variable is used.

``APPEND``
  Mark ``Configure.xml`` for append to results previously submitted to a
  dashboard server since the last :command:`ctest_start` call.
  Append semantics are defined by the dashboard server in use.
  This does *not* cause results to be appended to a ``.xml`` file
  produced by a previous call to this command.

``OPTIONS <options>``
  Specify a :ref:`semicolon-separated list <CMake Language Lists>` of
  command-line arguments to pass to the configuration tool.
  This option is ignored when :variable:`CTEST_CONFIGURE_COMMAND` is used.

``PRESET <preset>``
  .. versionadded:: 4.4

  Specify a :manual:`preset <cmake-presets(7)>` to use when configuring the
  project.  Any value set in the CTest script will take priority over a
  corresponding setting from the preset.  For example, the
  :variable:`CTEST_BINARY_DIRECTORY` variable will override the
  :preset:`configurePresets.binaryDir` setting from the chosen preset.

  When a preset is specified, the :variable:`CTEST_CONFIGURE_COMMAND`
  variable is ignored so that the preset is always honored.

  See also the :variable:`CTEST_CONFIGURE_PRESET` and
  :variable:`CTEST_PRESET` variables.

``PRESETS_FILE <file>``
  .. versionadded:: 4.4

  Specify a :manual:`presets <cmake-presets(7)>` file to use instead of the
  default ``CMakePresets.json`` in the source directory.
  A relative path is interpreted relative to the source directory.
  Has no effect unless a preset is selected via the ``PRESET`` argument
  or the :variable:`CTEST_CONFIGURE_PRESET` or :variable:`CTEST_PRESET`
  variables.

  See also the :variable:`CTEST_PRESETS_FILE` variable.

``RETURN_VALUE <result-var>``
  Store in the ``<result-var>`` variable the return value of the native
  configuration tool.

``CAPTURE_CMAKE_ERROR <result-var>``
  .. versionadded:: 3.7

  Store in the ``<result-var>`` variable ``-1`` if there are any errors running
  the command and prevent :manual:`ctest(1)` from returning non-zero if an
  error occurs.

``QUIET``
  .. versionadded:: 3.3

  Suppress any CTest-specific non-error messages that would have
  otherwise been printed to the console.  Output from the underlying
  configure command is not affected.

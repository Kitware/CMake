ctest_build
-----------

Build the project.

::

  ctest_build([BUILD build_dir] [TARGET target] [RETURN_VALUE res]
              [APPEND][NUMBER_ERRORS val] [NUMBER_WARNINGS val])

Builds the given build directory and stores results in Build.xml.  If
no BUILD is given, the CTEST_BINARY_DIRECTORY variable is used.

The TARGET variable can be used to specify a build target.  If none is
specified, the "all" target will be built.

The RETURN_VALUE option specifies a variable in which to store the
return value of the native build tool.  The NUMBER_ERRORS and
NUMBER_WARNINGS options specify variables in which to store the number
of build errors and warnings detected.

The APPEND option marks results for append to those previously
submitted to a dashboard server since the last ctest_start.  Append
semantics are defined by the dashboard server in use.

If set, the contents of the variable CTEST_BUILD_FLAGS are passed as
additional arguments to the underlying build command. This can e.g. be
used to trigger a parallel build using the -j option of make. See
:module:`ProcessorCount` for an example.

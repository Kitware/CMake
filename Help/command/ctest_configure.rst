ctest_configure
---------------

Configure the project build tree.

::

  ctest_configure([BUILD build_dir] [SOURCE source_dir] [APPEND]
                  [OPTIONS options] [RETURN_VALUE res])

Configures the given build directory and stores results in
Configure.xml.  If no BUILD is given, the CTEST_BINARY_DIRECTORY
variable is used.  If no SOURCE is given, the CTEST_SOURCE_DIRECTORY
variable is used.  The OPTIONS argument specifies command line
arguments to pass to the configuration tool.  The RETURN_VALUE option
specifies a variable in which to store the return value of the native
build tool.

The APPEND option marks results for append to those previously
submitted to a dashboard server since the last ctest_start.  Append
semantics are defined by the dashboard server in use.

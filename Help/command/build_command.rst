build_command
-------------

Get the command line to build this project.

::

  build_command(<variable>
                [CONFIGURATION <config>]
                [PROJECT_NAME <projname>]
                [TARGET <target>])

Sets the given <variable> to a string containing the command line for
building one configuration of a target in a project using the build
tool appropriate for the current CMAKE_GENERATOR.

If CONFIGURATION is omitted, CMake chooses a reasonable default value
for multi-configuration generators.  CONFIGURATION is ignored for
single-configuration generators.

If PROJECT_NAME is omitted, the resulting command line will build the
top level PROJECT in the current build tree.

If TARGET is omitted, the resulting command line will build
everything, effectively using build target 'all' or 'ALL_BUILD'.

::

  build_command(<cachevariable> <makecommand>)

This second signature is deprecated, but still available for backwards
compatibility.  Use the first signature instead.

Sets the given <cachevariable> to a string containing the command to
build this project from the root of the build tree using the build
tool given by <makecommand>.  <makecommand> should be the full path to
msdev, devenv, nmake, make or one of the end user build tools.

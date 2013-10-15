cmake_minimum_required
----------------------

Set the minimum required version of cmake for a project.

::

  cmake_minimum_required(VERSION major[.minor[.patch[.tweak]]]
                         [FATAL_ERROR])

If the current version of CMake is lower than that required it will
stop processing the project and report an error.  When a version
higher than 2.4 is specified the command implicitly invokes

::

  cmake_policy(VERSION major[.minor[.patch[.tweak]]])

which sets the cmake policy version level to the version specified.
When version 2.4 or lower is given the command implicitly invokes

::

  cmake_policy(VERSION 2.4)

which enables compatibility features for CMake 2.4 and lower.

The FATAL_ERROR option is accepted but ignored by CMake 2.6 and
higher.  It should be specified so CMake versions 2.4 and lower fail
with an error instead of just a warning.

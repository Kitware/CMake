project
-------

Set a name for the entire project.

::

  project(<projectname> [VERSION major[.minor[.patch[.tweak]]]] [languageName1 languageName2 ... ] )

Sets the name of the project, the name is also stored in the PROJECT_NAME
variable.  Additionally this sets the cache variables
<projectName>_BINARY_DIR and <projectName>_SOURCE_DIR to the
respective values, as well as the PROJECT_BINARY_DIR and PROJECT_SOURCE_DIR
variables.

If a version is specified, the :command:`project()` command sets the variables
PROJECT_VERSION and <projectName>_VERSION to this version.
Additionally the up to 4 components of the version string are stored in the
PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH and
PROJECT_VERSION_TWEAK, as well as the <projectName>_VERSION_MAJOR,
<projectName>_VERSION_MINOR, <projectName>_VERSION_PATCH> and
<projectName>_VERSION_TWEAK variables.
If VERSION is not used, the PROJECT_VERSION variables will be unset if they
have been set by a :command:`project(VERSION)` call before. This can be disabled
by setting the variable CMAKE_PROJECT_VERSION_SET_BY_PROJECT_COMMAND to FALSE.

Optionally you can specify which languages your project supports.
Example languages are CXX (i.e.  C++), C, Fortran, etc.  By default C
and CXX are enabled.  E.g.  if you do not have a C++ compiler, you can
disable the check for it by explicitly listing the languages you want
to support, e.g.  C.  By using the special language "NONE" all checks
for any language can be disabled.  If a variable exists called
CMAKE_PROJECT_<projectName>_INCLUDE, the file pointed to by that
variable will be included as the last step of the project command.

The top-level CMakeLists.txt file for a project must contain a
literal, direct call to the project() command; loading one through the
include() command is not sufficient.  If no such call exists CMake
will implicitly add one to the top that enables the default languages
(C and CXX).

project
-------

Set a name for the entire project.

::

  project(<projectname> [languageName1 languageName2 ... ] )

Sets the name of the project.  Additionally this sets the variables
<projectName>_BINARY_DIR and <projectName>_SOURCE_DIR to the
respective values.

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

project
-------

Set a name, version, and enable languages for the entire project.

.. code-block:: cmake

 project(<PROJECT-NAME>
         [VERSION <major>[.<minor>[.<patch>[.<tweak>]]]]
         [<language-name>...])

Sets the name of the project and stores the name in the
:variable:`PROJECT_NAME` variable.  Additionally this sets variables

* :variable:`PROJECT_SOURCE_DIR`,
  :variable:`<PROJECT-NAME>_SOURCE_DIR`
* :variable:`PROJECT_BINARY_DIR`,
  :variable:`<PROJECT-NAME>_BINARY_DIR`

If ``VERSION`` is specified, the components must be non-negative integers.
The :command:`project()` command stores the version number and its components
in variables

* :variable:`PROJECT_VERSION`,
  :variable:`<PROJECT-NAME>_VERSION`
* :variable:`PROJECT_VERSION_MAJOR`,
  :variable:`<PROJECT-NAME>_VERSION_MAJOR`
* :variable:`PROJECT_VERSION_MINOR`,
  :variable:`<PROJECT-NAME>_VERSION_MINOR`
* :variable:`PROJECT_VERSION_PATCH`,
  :variable:`<PROJECT-NAME>_VERSION_PATCH`
* :variable:`PROJECT_VERSION_TWEAK`,
  :variable:`<PROJECT-NAME>_VERSION_TWEAK`

If ``VERSION`` is not used, the :variable:`PROJECT_VERSION` variables will be
unset if they have been set by a :command:`project(VERSION)` call before.
(This can be disabled by setting the variable
``CMAKE_PROJECT_VERSION_SET_BY_PROJECT_COMMAND`` to ``FALSE`` before
invoking the command.)

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

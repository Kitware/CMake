INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
------------------------------------

List of public system include directories for a library.

Targets may populate this property to publish the include directories
which contain system headers, and therefore should not result in
compiler warnings.  The :command:`target_include_directories(SYSTEM)`
command signature populates this property with values given to the
``PUBLIC`` and ``INTERFACE`` keywords.  Projects may also get and set the
property directly.

When target dependencies are specified using :command:`target_link_libraries`,
CMake will read this property from all target dependencies to mark the
same include directories as containing system headers.

Contents of ``INTERFACE_SYSTEM_INCLUDE_DIRECTORIES`` may use "generator
expressions" with the syntax ``$<...>``.  See the
:manual:`cmake-generator-expressions(7)` manual for available expressions.
See the :manual:`cmake-buildsystem(7)` manual for more on defining
buildsystem properties.

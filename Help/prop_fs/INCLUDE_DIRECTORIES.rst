INCLUDE_DIRECTORIES
-------------------

.. versionadded:: 4.4

List of preprocessor include file search directories.

This property holds a :ref:`semicolon-separated list <CMake Language Lists>` of paths
and will be added to the list of include directories when the sources of this
file set are built. These directories will take precedence over directories
defined at target level and source level except for :generator:`Xcode`
generator due to technical limitations.

Relative paths should not be added to this property directly.

Contents of ``INCLUDE_DIRECTORIES`` may use "generator expressions" with
the syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)` manual
for available expressions.  However, :generator:`Xcode` does not support
per-config per-source settings, so expressions that depend on the build
configuration are not allowed with that generator.

Related properties:

* Use :prop_fs:`COMPILE_DEFINITIONS` to pass additional preprocessor definitions.
* Use :prop_fs:`COMPILE_OPTIONS` to pass additional compile flags.

Related commands:

* :command:`include_directories` for directory-wide settings
* :command:`target_include_directories` for target-specific settings
* :command:`set_source_files_properties` for source-specific settings

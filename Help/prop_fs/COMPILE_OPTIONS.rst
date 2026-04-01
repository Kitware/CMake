COMPILE_OPTIONS
---------------

.. versionadded:: 4.4

List of additional options to pass to the compiler.

This property holds a :ref:`semicolon-separated list <CMake Language Lists>`
of options and will be added to the list of compile flags when the sources of
this file set are built.  The options will be added after target-wide options
and source level ones.

Contents of ``COMPILE_OPTIONS`` may use "generator expressions" with the
syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)` manual
for available expressions.  However, :generator:`Xcode`
does not support per-config per-source settings, so expressions
that depend on the build configuration are not allowed with that
generator.

Usage example:

.. code-block:: cmake

  set_property(FILE_SET SOURCES TARGET foo PROPERTY
    COMPILE_OPTIONS "-Wno-unused-parameter;-Wno-missing-field-initializer")

Related properties:

* Use :prop_fs:`COMPILE_DEFINITIONS` to pass additional preprocessor definitions.
* Use :prop_fs:`INCLUDE_DIRECTORIES` to pass additional include directories.

Related commands:

* :command:`add_compile_options` for directory-wide settings
* :command:`target_compile_options` for target-specific settings
* :command:`set_source_files_properties` for source-specific settings

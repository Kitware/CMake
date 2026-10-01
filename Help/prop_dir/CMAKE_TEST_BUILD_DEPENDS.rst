CMAKE_TEST_BUILD_DEPENDS
------------------------

.. versionadded:: 4.5

List of directory-scoped build time dependencies for tests.

This property holds a
:ref:`semicolon-separated list <CMake Language Lists>` of targets and/or files
that should be built by ``test_prep/all`` when
:variable:`CMAKE_TEST_BUILD_DEPENDS` is enabled.

Projects may append to this property directly with :command:`set_property` or
:command:`set_directory_properties` to extend ``test_prep/all`` with
directory-scoped build dependencies.

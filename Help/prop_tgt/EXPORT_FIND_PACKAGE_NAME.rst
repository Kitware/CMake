EXPORT_FIND_PACKAGE_NAME
------------------------

.. note::

  Experimental. Gated by ``CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES``.

Control the package name associated with a dependency target when exporting a
:command:`find_dependency` call in :command:`install(EXPORT)` or
:command:`export(EXPORT)`. This can be used to assign a package name to a
package that is built by CMake and exported, or to override the package in the
:command:`find_package` call that created the target.

This property is initialized by :variable:`CMAKE_EXPORT_FIND_PACKAGE_NAME`.

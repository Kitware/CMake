CMake Experimental Features Guide
*********************************

The following is a guide to CMake experimental features that are
under development and not yet included in official documentation.
See documentation on `CMake Development`_ for more information.

.. _`CMake Development`: README.rst

Features are gated behind ``CMAKE_EXPERIMENTAL_`` variables which must be set
to specific values in order to enable their gated behaviors. Note that the
specific values will change over time to reinforce their experimental nature.
When used, a warning will be generated to indicate that an experimental
feature is in use and that the affected behavior in the project is not part of
CMake's stability guarantees.

Export Package Dependencies
===========================

In order to activate support for this experimental feature, set

* variable ``CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES`` to
* value ``1942b4fa-b2c5-4546-9385-83f254070067``.

This UUID may change in future versions of CMake.  Be sure to use the value
documented here by the source tree of the version of CMake with which you are
experimenting.

When activated, this experimental feature provides the following:

* The ``install(EXPORT)`` and ``export(EXPORT)`` commands have experimental
  ``EXPORT_PACKAGE_DEPENDENCIES`` arguments to generate ``find_dependency``
  calls automatically.

* Details of the calls may be configured using the ``export(SETUP)``
  command's ``PACKAGE_DEPENDENCY`` argument.

* The package name associated with specific targets may be specified
  using the ``CMAKE_EXPORT_FIND_PACKAGE_NAME`` variable and/or
``EXPORT_FIND_PACKAGE_NAME`` target property.

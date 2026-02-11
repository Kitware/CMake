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

Export |CPS| Package Information for ``install(EXPORT)``
========================================================

In order to activate support for this experimental feature, set

* variable ``CMAKE_EXPERIMENTAL_MAPPED_PACKAGE_INFO`` to
* value ``ababa1b5-7099-495f-a9cd-e22d38f274f2``.

These UUIDs may change in future versions of CMake.  Be sure to use the values
documented here by the source tree of the version of CMake with which you are
experimenting.

When activated, this experimental feature provides the following:

* Setting ``CMAKE_INSTALL_EXPORTS_AS_PACKAGE_INFO`` enables generation of
  package information in the |CPS|_ format via the ``install(EXPORT)`` command.

Build database support
======================

In order to activate support for exporting build databases, set

* variable ``CMAKE_EXPERIMENTAL_EXPORT_BUILD_DATABASE`` to
* value ``73194a1d-c0b5-41b9-9190-a4512925e192``.

This UUID may change in future versions of CMake.  Be sure to use the value
documented here by the source tree of the version of CMake with which you are
experimenting.

When activated, this experimental feature provides the following:

* The :prop_tgt:`EXPORT_BUILD_DATABASE` target property and its initializing
  variable :variable:`CMAKE_EXPORT_BUILD_DATABASE` and environment variable
  :envvar:`CMAKE_EXPORT_BUILD_DATABASE`.

* Targets with the property set to a true value will have their C++ build
  information exported to the build database.

Software Bill Of Materials |SBOM|
=================================

In order to activate support for the :command:`install(SBOM)` command,
set

* variable ``CMAKE_EXPERIMENTAL_GENERATE_SBOM`` to
* value ``ca494ed3-b261-4205-a01f-603c95e4cae0``.

This UUID may change in future versions of CMake.  Be sure to use the value
documented here by the source tree of the version of CMake with which you are
experimenting.

When activated, this experimental feature provides the following:

* The experimental ``export(SBOM)`` and ``install(SBOM)`` commands are
  available to generate a Software Bill of Materials or "SBOM" for the current
  project. See :command:`install(SBOM)` for a complete overview of the command.

Rust Support
============

In order to activate support for Rust, set

* variable ``CMAKE_EXPERIMENTAL_RUST`` to
* value ``3cc9b32c-47d3-4056-8953-d74e69fc0d6c``.

This UUID may change in future versions of CMake.  Be sure to use the value
documented here by the source tree of the version of CMake with which you are
experimenting.

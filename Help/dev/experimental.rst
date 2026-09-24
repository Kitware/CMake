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

.. _CPS: https://cps-org.github.io/cps/
.. |CPS| replace:: Common Package Specification

Build database support
======================

In order to activate support for exporting build databases, set

* variable ``CMAKE_EXPERIMENTAL_EXPORT_BUILD_DATABASE`` to
* value ``70ef007e-b743-492d-9407-e35eeac03a40``.

This UUID may change in future versions of CMake.  Be sure to use the value
documented here by the source tree of the version of CMake with which you are
experimenting.

When activated, this experimental feature provides the following:

* The :prop_tgt:`EXPORT_BUILD_DATABASE` target property and its initializing
  variable :variable:`CMAKE_EXPORT_BUILD_DATABASE` and environment variable
  :envvar:`CMAKE_EXPORT_BUILD_DATABASE`.

* Targets with the property set to a true value will have their C++ build
  information exported to the build database.

Rust Support
============

In order to activate support for Rust, set

* variable ``CMAKE_EXPERIMENTAL_RUST`` to
* value ``b6fdddce-bf66-41a5-bc5f-077f6fa4d2a1``.

This UUID may change in future versions of CMake.  Be sure to use the value
documented here by the source tree of the version of CMake with which you are
experimenting.

SYCL Support
============

In order to activate support for SYCL, set

* variable ``CMAKE_EXPERIMENTAL_SYCL`` to
* value ``c0d1fb10-2ece-420e-9d29-7d7f2b300f25``.

This UUID may change in future versions of CMake.  Be sure to use the value
documented here by the source tree of the version of CMake with which you are
experimenting.

When activated, this experimental feature provides the following:

* The experimental ``SYCL`` language, including ``enable_language(SYCL)`` and
  ``project(... LANGUAGES SYCL)``.

* Target properties and variables for SYCL language standard selection.

* The ``SYCL_EXTENSION_MODE`` target property, which controls whether SYCL
  sources use only SYCL language flags and feature requirements or prepend C++
  language flags and combine C++ feature and standard requirements.

* The ``SYCL`` and ``SYCL_HEADERS`` file set type aliases, which initialize the
  :prop_fs:`LANGUAGE` property.  ``SYCL`` selects the language of a ``SOURCES``
  file set, while ``SYCL_HEADERS`` marks a ``HEADERS`` file set as requiring
  SYCL for C++ sources that can reach it.

The SYCL language is supported by Makefile, Ninja, FASTBuild, and Visual Studio
generators with AdaptiveCpp, SYCL-capable Clang, or IntelLLVM compiler drivers.

:prop_tgt:`SYCL_SEPARABLE_COMPILATION` controls separate compilation of device
code.  :prop_tgt:`SYCL_RESOLVE_DEVICE_SYMBOLS` controls device linking, including
for static libraries used by C++-only consumers.  See these properties for
supported compiler and generator configurations.

COMPILE_DEFINITIONS
-------------------

Preprocessor definitions for compiling a source file.

The ``COMPILE_DEFINITIONS`` property may be set to a semicolon-separated
list of preprocessor definitions using the syntax ``VAR`` or ``VAR=value``.
Function-style definitions are not supported.  CMake will
automatically escape the value correctly for the native build system
(note that CMake language syntax may require escapes to specify some
values).  This property may be set on a per-configuration basis using
the name ``COMPILE_DEFINITIONS_<CONFIG>`` where ``<CONFIG>`` is an upper-case
name (ex.  ``COMPILE_DEFINITIONS_DEBUG``).

CMake will automatically drop some definitions that are not supported
by the native build tool.  :generator:`Xcode` does not support per-configuration
definitions on source files.

.. versionadded:: 3.26
  Any leading ``-D`` on an item will be removed.

.. include:: /include/COMPILE_DEFINITIONS_DISCLAIMER.rst

.. |genex_subject| replace:: ``COMPILE_DEFINITIONS``
.. include:: ../include/GENEX_NOTE_PER_SOURCE.rst

Prefer using generator expressions in :prop_sf:`!COMPILE_DEFINITIONS` over the
deprecated :prop_sf:`COMPILE_DEFINITIONS_<CONFIG>` property.

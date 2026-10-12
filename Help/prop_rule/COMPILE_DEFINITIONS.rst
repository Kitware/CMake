COMPILE_DEFINITIONS
-------------------

.. versionadded:: 4.5

Preprocessor definitions for compiling the file set's sources associated with
this rule.

The ``COMPILE_DEFINITIONS`` property may be set to a
:ref:`semicolon-separated list <CMake Language Lists>` of preprocessor
definitions using the syntax ``VAR`` or ``VAR=value``. Function-style
definitions are not supported.  CMake will automatically escape the value
correctly for the native build system (note that CMake language syntax may
require escapes to specify some values).

CMake will automatically drop definitions that are not supported
by the native build tool.

.. include:: /include/COMPILE_DEFINITIONS_DISCLAIMER.rst

.. |genex_subject| replace:: ``COMPILE_DEFINITIONS``
.. include:: ../include/GENEX_NOTE.rst

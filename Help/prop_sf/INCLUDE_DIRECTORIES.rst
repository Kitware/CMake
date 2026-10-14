INCLUDE_DIRECTORIES
-------------------

.. versionadded:: 3.11

List of preprocessor include file search directories.

This property holds a :ref:`semicolon-separated list <CMake Language Lists>` of paths
and will be added to the list of include directories when this
source file builds. These directories will take precedence over directories
defined at target level except for :generator:`Xcode` generator due to technical
limitations.

Relative paths should not be added to this property directly.

.. |genex_subject| replace:: ``INCLUDE_DIRECTORIES``
.. include:: ../include/GENEX_NOTE_PER_SOURCE.rst

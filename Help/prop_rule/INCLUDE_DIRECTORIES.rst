INCLUDE_DIRECTORIES
-------------------

.. versionadded:: 4.5

List of preprocessor include file search directories.

The ``INCLUDE_DIRECTORIES`` property may be set to a
:ref:`semicolon-separated list <CMake Language Lists>` of directories given so
far to the :command:`set_property(RULE)` command.

The value of this property is used by the rule definitions to set the include
paths for the compiler.

Relative paths should not be added to this property.

.. |genex_subject| replace:: ``INCLUDE_DIRECTORIES``
.. include:: ../include/GENEX_NOTE.rst

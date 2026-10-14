INCLUDE_DIRECTORIES
-------------------

List of preprocessor include file search directories.

This property specifies the list of directories given so far to the
:command:`target_include_directories` command.  In addition to accepting
values from that command, values may be set directly on any
target using the :command:`set_property` command.  A target gets its
initial value for this property from the value of the
:prop_dir:`INCLUDE_DIRECTORIES` directory property.  Both directory and
target property values are adjusted by calls to the
:command:`include_directories` command.

The value of this property is used by the generators to set the include
paths for the compiler.

Relative paths should not be added to this property directly. Use one of
the commands above instead to handle relative paths.

.. |genex_subject| replace:: ``INCLUDE_DIRECTORIES``
.. include:: ../include/GENEX_NOTE.rst

SOURCE_SETS
-----------

.. versionadded:: 4.4

Read-only list of the target's ``PRIVATE`` and ``PUBLIC`` source sets (i.e.
all file sets with the type ``SOURCES``). Files listed in these file sets are
treated as source files.

Source sets may be defined using the :command:`target_sources` command
``FILE_SET`` option with type ``SOURCES``.

See also :prop_tgt:`SOURCE_SET_<NAME>`, :prop_tgt:`SOURCE_SET` and
:prop_tgt:`INTERFACE_SOURCE_SETS`.

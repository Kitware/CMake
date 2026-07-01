SOURCE_SETS
-----------

.. versionadded:: 4.4

Read-only list of the target's ``PRIVATE`` and ``PUBLIC`` source sets (i.e.
all file sets with the type ``SOURCES``). Files listed in these file sets are
treated as source files.

See Also
^^^^^^^^

Related properties:

* :prop_tgt:`FILE_SETS_<TYPE>`
* :prop_tgt:`INTERFACE_FILE_SETS_<TYPE>`
* :prop_tgt:`INTERFACE_SOURCE_SETS`
* :prop_tgt:`SOURCE_SET_<NAME>`
* :prop_tgt:`SOURCE_SET`

Related commands:

* :command:`target_sources` to define file sets of type ``SOURCES``

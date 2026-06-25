HEADER_SETS
-----------

.. versionadded:: 3.23

Read-only list of the target's ``PRIVATE`` and ``PUBLIC`` header sets (i.e.
all file sets with the type ``HEADERS``). Files listed in these file sets are
treated as source files for the purpose of IDE integration. The files also
have their :prop_sf:`HEADER_FILE_ONLY` property set to ``TRUE``.

See Also
^^^^^^^^

Related properties:

* :prop_tgt:`FILE_SETS_<TYPE>`
* :prop_tgt:`INTERFACE_FILE_SETS_<TYPE>`
* :prop_tgt:`INTERFACE_HEADER_SETS`
* :prop_tgt:`HEADER_SET_<NAME>`
* :prop_tgt:`HEADER_SET`

Related commands:

* :command:`target_sources` to define file sets of type ``HEADERS``

CXX_MODULE_SETS
---------------

.. versionadded:: 3.28

Read-only list of the target's ``PRIVATE`` and ``PUBLIC`` C++ module sets (i.e.
all file sets with the type ``CXX_MODULES``). Files listed in these file sets
are treated as source files for the purpose of IDE integration.

See Also
^^^^^^^^

Related properties:

* :prop_tgt:`FILE_SETS_<TYPE>`
* :prop_tgt:`INTERFACE_FILE_SETS_<TYPE>`
* :prop_tgt:`INTERFACE_CXX_MODULE_SETS`
* :prop_tgt:`CXX_MODULE_SET_<NAME>`
* :prop_tgt:`CXX_MODULE_SET`

Related commands:

* :command:`target_sources` to define file sets of type ``CXX_MODULES``

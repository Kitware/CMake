INTERFACE_CXX_MODULE_SETS
-------------------------

.. versionadded:: 3.28

Read-only list of the target's ``PUBLIC`` C++ module sets (i.e. all file sets
with the type ``CXX_MODULES``). Files listed in these C++ module sets can be
installed with :command:`install(TARGETS)` and exported with
:command:`install(EXPORT)` and :command:`export`.

See Also
^^^^^^^^

Related properties:

* :prop_tgt:`INTERFACE_FILE_SETS_<TYPE>`
* :prop_tgt:`FILE_SETS_<TYPE>`
* :prop_tgt:`CXX_MODULE_SETS`

Related commands:

* :command:`target_sources` to define file sets of type ``CXX_MODULES``

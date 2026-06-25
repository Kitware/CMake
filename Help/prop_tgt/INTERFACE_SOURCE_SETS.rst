INTERFACE_SOURCE_SETS
---------------------

.. versionadded:: 4.4

Read-only list of the target's ``INTERFACE`` and ``PUBLIC`` source sets (i.e.
all file sets with the type ``SOURCES``). Files listed in these source sets
can be installed with :command:`install(TARGETS)` and exported with
:command:`install(EXPORT)` and :command:`export`.

See Also
^^^^^^^^

Related properties:

* :prop_tgt:`INTERFACE_FILE_SETS_<TYPE>`
* :prop_tgt:`FILE_SETS_<TYPE>`
* :prop_tgt:`SOURCE_SETS`

Related commands:

* :command:`target_sources` to define file sets of type ``SOURCES``

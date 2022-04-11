INTERFACE_HEADER_SETS
---------------------

.. versionadded:: 3.23

Read-only list of the target's ``INTERFACE`` and ``PUBLIC`` header sets (i.e.
all file sets with the type ``HEADERS``). Files listed in these header sets
can be installed with :command:`install(TARGETS)` and exported with
:command:`install(EXPORT)` and :command:`export`.

See also :prop_tgt:`HEADER_SETS`.

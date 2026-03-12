INTERFACE_SOURCE_SETS
---------------------

.. versionadded:: 4.4

Read-only list of the target's ``INTERFACE`` and ``PUBLIC`` source sets (i.e.
all file sets with the type ``SOURCES``). Files listed in these source sets
can be installed with :command:`install(TARGETS)` and exported with
:command:`install(EXPORT)` and :command:`export`.

Source sets may be defined using the :command:`target_sources` command
``FILE_SET`` option with type ``SOURCES``.

See also :prop_tgt:`SOURCE_SETS`.

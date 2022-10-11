INTERFACE_CXX_MODULE_HEADER_UNIT_SETS
-------------------------------------

.. versionadded:: 3.25

.. note ::

  Experimental. Gated by ``CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API``

Read-only list of the target's ``PUBLIC`` C++ module header sets (i.e. all
file sets with the type ``CXX_MODULE_HEADER_UNITS``). Files listed in these
C++ module header sets can be installed with :command:`install(TARGETS)` and
exported with :command:`install(EXPORT)` and :command:`export`.

C++ module header sets may be defined using the :command:`target_sources`
command ``FILE_SET`` option with type ``CXX_MODULE_HEADER_UNITS``.

See also :prop_tgt:`CXX_MODULE_HEADER_UNIT_SETS`.

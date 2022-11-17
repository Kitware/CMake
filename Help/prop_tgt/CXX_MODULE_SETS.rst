CXX_MODULE_SETS
---------------

.. versionadded:: 3.25

.. note ::

  Experimental. Gated by ``CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API``

Read-only list of the target's ``PRIVATE`` and ``PUBLIC`` C++ module sets (i.e.
all file sets with the type ``CXX_MODULES``). Files listed in these file sets
are treated as source files for the purpose of IDE integration.

C++ module sets may be defined using the :command:`target_sources` command
``FILE_SET`` option with type ``CXX_MODULES``.

See also :prop_tgt:`CXX_MODULE_SET_<NAME>`, :prop_tgt:`CXX_MODULE_SET` and
:prop_tgt:`INTERFACE_CXX_MODULE_SETS`.

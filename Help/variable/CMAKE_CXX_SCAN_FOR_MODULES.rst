CMAKE_CXX_SCAN_FOR_MODULES
--------------------------

.. versionadded:: 3.26

Whether to scan C++ source files for module dependencies.

This variable is used to initialize the :prop_tgt:`CXX_SCAN_FOR_MODULES`
property on all the targets.  See that target property for additional
information.

.. note ::

  This setting is meaningful only when experimental support for C++ modules
  has been enabled by the ``CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API`` gate.

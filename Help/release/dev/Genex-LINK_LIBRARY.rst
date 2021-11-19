Genex-LINK_LIBRARY
------------------

* The :genex:`LINK_LIBRARY` generator expression was added to manage how
  libraries are specified during the link step. The variables
  :variable:`CMAKE_<LANG>_LINK_USING_<FEATURE>` and
  :variable:`CMAKE_LINK_USING_<FEATURE>` are used to define features usable by
  the :genex:`LINK_LIBRARY` generator expression. Moreover, the
  :prop_tgt:`LINK_LIBRARY_OVERRIDE` and
  :prop_tgt:`LINK_LIBRARY_OVERRIDE_<LIBRARY>` target properties are available
  to resolve incompatible features.

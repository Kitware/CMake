interface-compile-link-wrappers
-------------------------------

* The :prop_gbl:`CMAKE_COMPILE_WRAPPER_<NAME>` and
  :prop_gbl:`CMAKE_LINK_WRAPPER_<NAME>` global properties were added alongside
  the :prop_tgt:`COMPILE_WRAPPERS`, :prop_tgt:`LINK_WRAPPERS`,
  :prop_tgt:`INTERFACE_COMPILE_WRAPPERS`, and
  :prop_tgt:`INTERFACE_LINK_WRAPPERS` target properties. Together, for the
  :ref:`Makefile Generators` and :ref:`Ninja Generators`, they support wrapping
  compile and link commands for a given target or its consumers with arbitrary
  command lines.

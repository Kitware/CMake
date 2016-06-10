find-lib32
----------

* The :command:`find_library` and :command:`find_package` commands learned
  to search in ``lib32/`` directories when the build targets a 32-bit
  architecture.  See the :prop_gbl:`FIND_LIBRARY_USE_LIB32_PATHS` global
  property.

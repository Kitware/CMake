imported-implib-only
--------------------

* On imported shared libraries, the :prop_tgt:`IMPORTED_IMPLIB` target
  property may now be used without :prop_tgt:`IMPORTED_LOCATION`.
  This can be used to represent a stub library whose location should not
  be added as a runtime search path to dependents that link it.

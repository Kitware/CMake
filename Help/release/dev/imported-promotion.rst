imported-promotion
------------------

* Added new target-property :prop_tgt:`IMPORTED_GLOBAL` which
  indicates if an :ref:`IMPORTED target <Imported Targets>` is
  globally visible.
  It will be set automatically if such an imported target is
  created with the ``GLOBAL`` flag.

* Additionally, it is now also possible to promote a local imported
  target to become globally visible by setting its
  :prop_tgt:`IMPORTED_GLOBAL` property to `TRUE`. (However, this
  promotion can only succeed if it is done from within the same
  directory where the imported target was created in the first
  place.) Setting it to `FALSE` is not supported!

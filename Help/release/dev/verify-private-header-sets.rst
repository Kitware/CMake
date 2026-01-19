verify-private-header-sets
--------------------------

* When :prop_tgt:`VERIFY_INTERFACE_HEADER_SETS` is set to true on an executable
  target, that target's interface file sets are verified regardless of its
  :prop_tgt:`ENABLE_EXPORTS` property. See policy :policy:`CMP0209`.
* The :variable:`CMAKE_VERIFY_PRIVATE_HEADER_SETS` variable and corresponding
  :prop_tgt:`VERIFY_PRIVATE_HEADER_SETS` target property were added to
  enable build rules that verify all headers in private file sets can be used
  on their own.
* The :prop_tgt:`PRIVATE_HEADER_SETS_TO_VERIFY` target property was added to
  customize which private file sets to verify when the target's
  :prop_tgt:`VERIFY_PRIVATE_HEADER_SETS` property is true.

always-create-all-verify-targets
---------------------------------

* The :prop_tgt:`VERIFY_INTERFACE_HEADER_SETS` and
  :prop_tgt:`VERIFY_PRIVATE_HEADER_SETS` target properties now always
  create the per-target verification target and the aggregate
  ``all_verify_interface_header_sets``, ``all_verify_private_header_sets``,
  and ``all_verify_header_sets`` targets when the property is enabled,
  even if the target has no matching header sets.

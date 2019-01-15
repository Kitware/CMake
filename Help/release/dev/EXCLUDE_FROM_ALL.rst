EXCLUDE_FROM_ALL
----------------

* A target's :prop_tgt:`EXCLUDE_FROM_ALL` property can now override the
  setting of its directory. A target will now be built as part of "all"
  if its :prop_tgt:`EXCLUDE_FROM_ALL` property is set to ``OFF``, even if its
  containing directory is marked as :prop_dir:`EXCLUDE_FROM_ALL`.

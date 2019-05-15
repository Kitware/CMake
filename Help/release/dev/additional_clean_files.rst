additional_clean_files
----------------------

* New target property :prop_tgt:`ADDITIONAL_CLEAN_FILES` and directory property
  :prop_dir:`ADDITIONAL_CLEAN_FILES` were added.  They allow to register
  additional files that should be removed during the clean stage.

* Directory property :prop_dir:`ADDITIONAL_MAKE_CLEAN_FILES` was marked
  deprecated.  The new directory property :prop_dir:`ADDITIONAL_CLEAN_FILES`
  should be used instead.

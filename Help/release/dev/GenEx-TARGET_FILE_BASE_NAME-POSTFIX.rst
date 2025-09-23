GenEx-TARGET_FILE_BASE_NAME-POSTFIX
-----------------------------------

* The :genex:`TARGET_FILE_BASE_NAME`, :genex:`TARGET_IMPORT_FILE_BASE_NAME`,
  :genex:`TARGET_LINKER_FILE_BASE_NAME`,
  :genex:`TARGET_LINKER_LIBRARY_FILE_BASE_NAME`,
  and :genex:`TARGET_LINKER_IMPORT_FILE_BASE_NAME`
  generator expressions gained the option ``POSTFIX`` to control the inclusion
  or not of the :prop_tgt:`<CONFIG>_POSTFIX` target property as part of the
  base name of the target.

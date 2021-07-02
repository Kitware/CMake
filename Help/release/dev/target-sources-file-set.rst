target-headers
--------------

* The :command:`target_sources` command gained a new ``FILE_SET`` mode, which
  can be used to add headers as header-only source files of a target.
* New :prop_tgt:`HEADER_SETS` and :prop_tgt:`INTERFACE_HEADER_SETS` properties
  were added, which list the header file sets associated with a target.
* New :prop_tgt:`HEADER_SET` and :prop_tgt:`HEADER_SET_<NAME>` properties were
  added, which list the files in the associated header file set.
* New :prop_tgt:`HEADER_DIRS` and :prop_tgt:`HEADER_DIRS_<NAME>` properties
  were added, which specify the base directories of the associated header file
  set.
* The :command:`install(TARGETS)` command gained a new ``FILE_SET`` argument,
  which can be used to install header file sets associated with a target.
* The :manual:`File API <cmake-file-api(7)>` ``codemodel-v2`` minor version has
  been bumped to ``4``.
* The :manual:`File API <cmake-file-api(7)>` ``codemodel-v2`` ``directory``
  object gained a new installer type of ``fileSet``.

system
------

* The :prop_tgt:`SYSTEM` target property was added to specify
  that a target should be treated as a system library (i.e.
  its include directories are automatically ``SYSTEM`` when
  compiling consumers).

* The :prop_dir:`SYSTEM` directory property was added to initialize the
  :prop_tgt:`SYSTEM` target property for targets created in that directory.

* The :command:`add_subdirectory` command gained a ``SYSTEM`` option
  to enable the :prop_dir:`SYSTEM` directory property in the subdirectory.

* The :module:`FetchContent` module :command:`FetchContent_Declare`
  command gained a ``SYSTEM`` option to enable the :prop_dir:`SYSTEM`
  directory property in the subdirectory.

* The :prop_tgt:`EXPORT_NO_SYSTEM` target property was added to
  specify that :command:`install(EXPORT)` and :command:`export`
  commands will generate a imported target with
  :prop_tgt:`SYSTEM` property `OFF`.

* The :prop_tgt:`IMPORTED_NO_SYSTEM` target property was deprecated
  in favor of :prop_tgt:`SYSTEM` and :prop_tgt:`EXPORT_NO_SYSTEM`.

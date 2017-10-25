defer-target-source-check
-------------------------

* :command:`add_library` and :command:`add_executable` commands can now be
  called without any sources and will not complain as long as sources will
  be added later via :command:`target_sources`.

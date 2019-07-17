aix
---

* On AIX, executables using the :prop_tgt:`ENABLE_EXPORTS` target property
  now produce a linker import file with a ``.imp`` extension in addition
  to the executable file.  Plugins (created via :command:`add_library` with
  the ``MODULE`` option) that use :command:`target_link_libraries` to link
  to the executable for its symbols are now linked using the import file.
  The :command:`install(TARGETS)` command now installs the import file as
  an ``ARCHIVE`` artifact.

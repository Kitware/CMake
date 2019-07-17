aix
---

* On AIX, executables using the :prop_tgt:`ENABLE_EXPORTS` target property
  now produce a linker import file with a ``.imp`` extension in addition
  to the executable file.  Plugins (created via :command:`add_library` with
  the ``MODULE`` option) that use :command:`target_link_libraries` to link
  to the executable for its symbols are now linked using the import file.
  The :command:`install(TARGETS)` command now installs the import file as
  an ``ARCHIVE`` artifact.

* On AIX, runtime linking is no longer enabled by default.  CMake provides
  the linker enough information to resolve all symbols up front.
  One may manually enable runtime linking for shared libraries and/or
  loadable modules by adding ``-Wl,-G`` to their link flags
  (e.g. in the :variable:`CMAKE_SHARED_LINKER_FLAGS` or
  :variable:`CMAKE_MODULE_LINKER_FLAGS` variable).
  One may manually enable runtime linking for executables by adding
  ``-Wl,-brtl`` to their link flags (e.g. in the
  :variable:`CMAKE_EXE_LINKER_FLAGS` variable).

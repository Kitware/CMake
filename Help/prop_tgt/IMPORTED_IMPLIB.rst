IMPORTED_IMPLIB
---------------

Full path to the import library for an ``IMPORTED`` target.

This property may be set:

* On DLL platforms, to the location of the ``.lib`` part of the DLL.
* On AIX, to an import file (e.g. ``.imp``) created for executables that export
  symbols (see the :prop_tgt:`ENABLE_EXPORTS` target property).
* On macOS, to an import file (e.g. ``.tbd``) created for shared libraries (see
  the :prop_tgt:`ENABLE_EXPORTS` target property). For frameworks this is the
  location of the ``.tbd`` file symlink just inside the framework folder.

This property is ignored for non-imported targets.

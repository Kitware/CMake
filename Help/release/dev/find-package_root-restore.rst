find-package_root-restore
-------------------------

* The :command:`find_package` command now searches a prefix specified by
  a ``PackageName_ROOT`` CMake or environment variable.  Package roots are
  maintained as a stack so nested calls to all ``find_*`` commands inside
  find modules also search the roots as prefixes.
  See policy :policy:`CMP0074`.

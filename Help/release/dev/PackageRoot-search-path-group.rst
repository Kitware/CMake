PackageRoot search path group
-----------------------------

* All ``find_`` commands now have a `PACKAGE_ROOT` search path group that is
  first in the search heuristics.  If the ``find_`` command is called from
  inside a find module, then the CMake and environment variables
  ``<PackageName>_ROOT`` are used as prefixes and are the first set of paths
  that are searched.

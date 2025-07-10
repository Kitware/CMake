find-package-UNWIND_INCLUDE
---------------------------

* The :command:`find_package()` command gained a new ``UNWIND_INCLUDE`` option
  to enable immediate :command:`return` from :command:`include()` commands
  after a failure to discover a transitive dependency.

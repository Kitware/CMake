find_package-global-imported
----------------------------

* The :command:`find_package` command gained a `GLOBAL` option that
  allows for the promotion of imported targets to global scope fur the
  duration of the :command:`find_package` call.

* Adds support for :variable:`CMAKE_FIND_PACKAGE_TARGETS_GLOBAL` to
  toggle behavior of the :command:`find_package` command's new GLOBAL option

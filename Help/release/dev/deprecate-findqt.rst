deprecate-findqt
----------------

* The :module:`FindQt` module is no longer used by the :command:`find_package`
  command as a find module.  This allows the Qt Project upstream to optionally
  provide its own ``QtConfig.cmake`` package configuration file and have
  applications use it via ``find_package(Qt)`` rather than
  ``find_package(Qt CONFIG)``.  See policy :policy:`CMP0084`.

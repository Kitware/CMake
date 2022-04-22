find-calls-search-install-prefix
--------------------------------

* The :command:`find_file`, :command:`find_library`, :command:`find_path`,
  :command:`find_package`, and :command:`find_program` commands have gained
  the `NO_CMAKE_INSTALL_PREFIX` option to control searching
  `CMAKE_INSTALL_PREFIX`.

* Adds support for :variable:`CMAKE_FIND_USE_INSTALL_PREFIX` to toggle
  behavior of the :command:`find_file`, :command:`find_library`, :command:`find_path`,
  :command:`find_package`, and :command:`find_program` commands new
  `NO_CMAKE_INSTALL_PREFIX` option.

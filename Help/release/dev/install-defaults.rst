install-defaults
----------------

* The ``TARGETS`` variant of the :command:`install` command learned how to
  install to an appropriate default directory for a given target type, based
  on variables from the :module:`GNUInstallDirs` module and built-in defaults,
  in lieu of a ``DESTINATION`` argument.
* The ``FILES`` and ``DIRECTORY`` variants of the :command:`install` command
  learned a new set of parameters for installing files as a file type, setting
  the destination based on the appropriate variables from
  :module:`GNUInstallDirs` and built-in defaults, in lieu of a ``DESTINATION``
  argument.

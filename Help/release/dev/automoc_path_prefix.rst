automoc_path_prefix
-------------------

* When using :prop_tgt:`AUTOMOC`, CMake now generates the ``-p`` path prefix
  option for ``moc``.  This ensures that ``moc`` output files are identical
  on different build setups (given, that the headers compiled by ``moc`` are
  in an :command:`include directory <target_include_directories>`).
  Also it ensures that ``moc`` output files will compile correctly when the
  source and/or build directory is a symbolic link.

  The ``moc`` path prefix generation behavior can be configured by setting
  the new :variable:`CMAKE_AUTOMOC_PATH_PREFIX` variable and/or
  :prop_tgt:`AUTOMOC_PATH_PREFIX` target property.

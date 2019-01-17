ExternalProject-non-cmake-source-subdir
---------------------------------------

* The :module:`ExternalProject` module's ``ExternalProject_Add`` command
  learned to apply ``SOURCE_SUBDIR`` when ``BUILD_IN_SOURCE`` is also used.
  The ``BUILD_COMMAND`` is run in the given ``SOURCE_SUBDIR`` of the
  ``SOURCE_DIR``.

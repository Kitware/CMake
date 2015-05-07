find_package-no-cmake-gui-paths
-------------------------------

* The :command:`find_package` command no longer considers project
  build trees recently configured in a :manual:`cmake-gui(1)`.
  This was previously done only on Windows and is now never done.
  The ``NO_CMAKE_BUILDS_PATH`` option is now ignored if given
  and effectively always on.
  Projects may populate the :ref:`User Package Registry` to aid
  users building multiple dependent projects one after another.

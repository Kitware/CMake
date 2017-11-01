write-single-xcodeproj
----------------------

* The :generator:`Xcode` generator behavior of generating one project
  file per :command:`project()` command could now be controlled with the
  :variable:`CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY` variable.
  This could be useful to speed up the CMake generation step for
  large projects and to work-around a bug in the ``ZERO_CHECK`` logic.

relative-rpath
--------------

* A :variable:`CMAKE_BUILD_RPATH_USE_ORIGIN` variable and corresponding
  :prop_tgt:`BUILD_RPATH_USE_ORIGIN` target property were added to
  enable use of relative runtime paths (RPATHs). This helps achieving
  relocatable and reproducible builds that are invariant of the build
  directory.

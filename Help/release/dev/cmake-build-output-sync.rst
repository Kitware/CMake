cmake-build-output-sync
-----------------------

* The :option:`cmake --build` command now groups the output of each recipe in
  a parallel build so that concurrent jobs do not interleave, when CMake is
  the one passing the parallel flag to the build tool (via :option:`--parallel
  <cmake--build --parallel>` or :envvar:`CMAKE_BUILD_PARALLEL_LEVEL`) and that
  tool is GNU Make 4.0 or newer.  Request parallelism natively, e.g.
  ``cmake --build . -- -j``, to opt out and stream output as it is produced.

cmake-qch-no-version-in-filename
--------------------------------

* The Qt Compressed Help file is now named ``CMake.qch``, which no longer
  contains the release version in the file name.  When CMake is upgraded
  in-place, the name and location of this file will remain constant.
  Tools such as IDEs, help viewers, etc. should now be able to refer to this
  file at a fixed location that remains valid across CMake upgrades.

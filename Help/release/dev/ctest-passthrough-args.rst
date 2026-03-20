ctest-passthrough-args
----------------------

* :manual:`ctest(1)` gained support for forwarding extra arguments to test
  executables using a :option:`-- <ctest -->` separator.

* :manual:`cmake-presets(7)` gained support for a
  ``testPassthroughArguments`` field in the
  test preset ``execution`` object for
  forwarding arguments to test executables via presets.

ctest-capture-error
-------------------

* The :command:`ctest_configure`, :command:`ctest_build`,
  :command:`ctest_test`, :command:`ctest_coverage`, and :command:`ctest_upload`
  commands gained a new ``CAPTURE_CMAKE_ERROR`` option to capture any errors
  that occur as the commands run into a variable and avoid affecting the return
  code of the :manual:`ctest(1)` process.

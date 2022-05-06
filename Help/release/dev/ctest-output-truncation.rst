ctest-output-truncation
-----------------------

* :manual:`ctest(1)` gained a ``--test-output-truncation`` option (and
  corresponding :variable:`CTEST_CUSTOM_TEST_OUTPUT_TRUNCATION` variable) to
  specify the truncation mode once the maximum test output size has been
  reached. Possible values are ``tail`` (default), ``middle`` or ``head``.
* :manual:`cmake-presets(7)` files now support schema version ``5``.
* :manual:`cmake-presets(7)` files gained support for specifying a
  ``testOutputTruncation`` field in test presets, which specifies the truncation
  mode once the maximum test output size has been reached.

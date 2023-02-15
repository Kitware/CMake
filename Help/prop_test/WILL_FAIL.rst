WILL_FAIL
---------

If set to true, this will invert the pass/fail flag of the test.

This property can be used for tests that are expected to fail and return a
non-zero return code. Note that system-level test failures such as segmentation
faults or heap errors will still fail the test even if ``WILL_FALL`` is true.

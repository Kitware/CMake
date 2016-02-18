TIMEOUT_AFTER_MATCH
-------------------

Change a test's timeout duration after a matching line is encountered
in its output.

Usage
^^^^^

.. code-block:: cmake

 add_test(mytest ...)
 set_property(TEST mytest PROPERTY TIMEOUT_AFTER_MATCH "${seconds}" "${regex}")

Description
^^^^^^^^^^^

The test's timeout duration is changed to ``seconds`` after it outputs
a line that matches ``regex``.  Prior to this, the timeout duration is
determined by the :prop_test:`TIMEOUT` property or the
:variable:`CTEST_TEST_TIMEOUT` variable if either of these are set.

:prop_test:`TIMEOUT_AFTER_MATCH` is useful for avoiding spurious
timeouts when your test must wait for some system resource to become
available before it can execute.  Set :prop_test:`TIMEOUT` to a longer
duration that accounts for resource acquisition and use
:prop_test:`TIMEOUT_AFTER_MATCH` to control how long the actual test
is allowed to run.

If the required resource can be controlled by CTest you should use
:prop_test:`RESOURCE_LOCK` instead of :prop_test:`TIMEOUT_AFTER_MATCH`.
This property should be used when only the test itself can determine
when its required resources are available.

SKIP_REGULAR_EXPRESSION
-----------------------

.. versionadded:: 3.16

If the output matches this regular expression the test will be marked as skipped.

If set, if the output matches one of specified regular expressions,
the test will be marked as skipped.  Example:

.. code-block:: cmake

  set_property(TEST mytest PROPERTY
    SKIP_REGULAR_EXPRESSION "[^a-z]Skip" "SKIP" "Skipped"
  )

``SKIP_REGULAR_EXPRESSION`` expects a list of regular expressions.

See also the :prop_test:`SKIP_RETURN_CODE`,
:prop_test:`PASS_REGULAR_EXPRESSION`, and :prop_test:`FAIL_REGULAR_EXPRESSION`
test properties.

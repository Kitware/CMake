add_skip_regular_expression_test_property
-----------------------------------------

* A new test property, :prop_test:`SKIP_REGULAR_EXPRESSION`, has been added.
  This property is similar to :prop_test:`FAIL_REGULAR_EXPRESSION` and
  :prop_test:`PASS_REGULAR_EXPRESSION`, but with the same meaning as
  :prop_test:`SKIP_RETURN_CODE`. This is useful, for example, in cases where
  the user has no control over the return code of the test. For example, in
  Catch2, the return value is the number of assertion failed, therefore it is
  impossible to use it for :prop_test:`SKIP_RETURN_CODE`.

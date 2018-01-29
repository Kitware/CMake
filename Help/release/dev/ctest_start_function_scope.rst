ctest_start_function_scope
--------------------------

* The :command:`ctest_start` command no longer sets
  :variable:`CTEST_RUN_CURRENT_SCRIPT` due to issues with scoping if it is
  called from inside a function. Instead, it sets an internal variable in
  CTest. However, setting :variable:`CTEST_RUN_CURRENT_SCRIPT` to 0 at the
  global scope still prevents the script from being re-run at the end.

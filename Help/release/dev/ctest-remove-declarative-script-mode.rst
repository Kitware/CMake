ctest-remove-declarative-script-mode
------------------------------------

* CTest's declarative scripting mode has been removed.  This mode used to be
  triggered by a :option:`ctest -S` script which did not call any
  :ref:`CTest Commands` unless :variable:`CTEST_RUN_CURRENT_SCRIPT` was
  explicitly set to ``OFF``.  This feature was undocumented and was not covered
  by any unit tests.

* The :variable:`CTEST_RUN_CURRENT_SCRIPT` variable no longer has any special
  meaning.

* The :command:`ctest_run_script` command may no longer be called without any
  arguments.

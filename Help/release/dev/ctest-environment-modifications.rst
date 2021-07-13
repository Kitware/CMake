ctest-environment-modifications
-------------------------------

* :manual:`ctest(1)` learned to be able to modify the environment for a test
  through the :prop_test:`ENVIRONMENT_MODIFICATION` property. This is allows
  for updates to environment variables based on the environment present at
  test time.

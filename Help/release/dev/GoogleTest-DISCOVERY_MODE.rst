GoogleTest-DISCOVERY_MODE
-------------------------

* The :module:`GoogleTest` module :command:`gtest_discover_tests` command
  gained a new ``DISCOVERY_MODE`` option to control when the test
  discovery step is run.  It offers a new ``PRE_TEST`` setting to
  run the discovery at test time instead of build time.

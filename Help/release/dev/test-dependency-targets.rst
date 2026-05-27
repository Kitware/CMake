test-build-dependencies
-----------------------

* The :ref:`Ninja Generators` now support generating a build target named
  ``test_prep/<test-name>`` for each test added by :command:`add_test`, which
  builds all dependencies for that test. Build dependencies are registered for
  an executable target invoked by the test, targets referenced in generator
  expressions in the test command, and explicit dependencies added using the
  new ``BUILD_DEPENDS`` option. This behavior is enabled by the new
  :variable:`CMAKE_TEST_BUILD_DEPENDS` variable. A ``test_prep/all``
  target is also generated to depend on every ``test_prep/<test-name>``
  target. Tests with names that are not valid target names are excluded from
  this behavior. If multiple tests in different directories share the same
  name, their dependencies are merged into one ``test_prep/<test-name>``
  target.

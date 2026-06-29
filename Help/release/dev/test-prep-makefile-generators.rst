test-prep-makefile-generators
-----------------------------

* The :generator:`FASTBuild` generator and the :ref:`Makefile Generators`
  now generate ``test_prep/<test-name>`` and ``test_prep/all`` convenience
  build targets when the :variable:`CMAKE_TEST_BUILD_DEPENDS` variable is
  enabled, matching the behavior of the :ref:`Ninja Generators`.  These
  targets build the dependencies of tests added by :command:`add_test`,
  including those listed with the ``BUILD_DEPENDS`` keyword.

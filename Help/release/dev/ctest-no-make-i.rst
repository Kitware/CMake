ctest-no-make-i
---------------

* The :command:`ctest_build` and :command:`build_command` commands
  no longer tell ``make`` tools to ignore errors with the ``-i`` option.
  Previously this was done for :ref:`Makefile Generators` but not others.
  See policy :policy:`CMP0061`.

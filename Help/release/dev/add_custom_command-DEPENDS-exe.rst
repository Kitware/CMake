add_custom_command-DEPENDS-exe
------------------------------

* Names of executables given to the ``DEPENDS`` argument of
  :command:`add_custom_command` no longer have ``.exe`` suffixes stripped to
  establish target-level dependencies. See policy :policy:`CMP0212`.

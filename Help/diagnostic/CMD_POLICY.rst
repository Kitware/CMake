CMD_POLICY
----------

.. versionadded:: 4.4

.. diagnostic::
  :default: warn
  :parent: CMD_AUTHOR

  Warn if a build system potentially depends on the ``OLD`` behavior of a CMake
  Policy.  See the :command:`cmake_policy` command for more details.  Setting a
  policy to ``OLD`` will also disable warnings for that specific policy.

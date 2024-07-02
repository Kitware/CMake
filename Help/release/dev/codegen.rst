codegen
-------

* The :ref:`Ninja Generators` and :ref:`Makefile Generators` now produce
  a ``codegen`` build target.  See policy :policy:`CMP0171`.  It drives a
  subset of the build graph sufficient to run custom commands created with
  :command:`add_custom_command`'s new ``CODEGEN`` option.

* The :command:`add_custom_command` command gained a ``CODEGEN`` option
  to mark a custom commands outputs as dependencies of a ``codegen`` target.

try_compile-signatures
----------------------

* The :command:`try_compile` and :command:`try_run` commands gained new
  signatures that more consistently use keyword dispatch and do not require a
  binary directory to be specified.  Additionally, these signatures use a
  unique directory for each invocation, which allows multiple outputs to be
  preserved when using :option:`--debug-trycompile <cmake --debug-trycompile>`.

* The :option:`cmake --debug-trycompile` option now prints log messages
  reporting the directory in which each try-compile check is done.

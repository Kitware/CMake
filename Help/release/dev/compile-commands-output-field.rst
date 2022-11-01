compile-commands-output-field
-----------------------------

* The :prop_tgt:`EXPORT_COMPILE_COMMANDS` target property will now have the
  ``output`` field in the compile commands objects. This allows multi-config
  generators (namely :generator:`Ninja Multi-Config` generator) to contain the
  compile commands for all configurations.

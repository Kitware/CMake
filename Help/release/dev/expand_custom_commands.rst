expand_custom_commands
----------------------

* The commands :command:`add_custom_command` and :command:`add_custom_target`
  learned the option ``COMMAND_EXPAND_LISTS`` which causes lists in the
  ``COMMAND`` argument to be expanded, including lists created by generator
  expressions.

custom-command-output-genex
---------------------------

* :command:`add_custom_command` and :command:`add_custom_target` now
  support :manual:`generator expressions <cmake-generator-expressions(7)>`
  in their ``OUTPUT`` and ``BYPRODUCTS`` options.

  Their ``COMMAND``, ``WORKING_DIRECTORY``, and ``DEPENDS`` options gained
  support for new generator expressions ``$<COMMAND_CONFIG:...>`` and
  ``$<OUTPUT_CONFIG:...>`` that control cross-config handling when using
  the :generator:`Ninja Multi-Config` generator.

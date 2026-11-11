COMMAND_CONFIG
--------------

Syntax
^^^^^^

.. code-block:: cmake

  $<COMMAND_CONFIG:...>

.. versionadded:: 3.20

Only valid in :command:`add_custom_command` and :command:`add_custom_target` as
the outer-most generator expression in an argument. With the :generator:`Ninja
Multi-Config` generator, generator expressions in ``...`` are evaluated using
the custom command's "command config". With other generators, the content of
``...`` is evaluated normally.

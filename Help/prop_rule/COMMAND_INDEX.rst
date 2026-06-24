COMMAND_<INDEX>
---------------

.. versionadded:: 4.5

Read-only property giving a
:ref:`semicolon-separated list <CMake Language Lists>` of the arguments of the
rule  as specified by the ``<INDEX>``th ``COMMAND`` option of the
:command:`add_custom_rule` command. The index range is starting at ``0``. If
the index specified is out of the range of commands, ``NOTFOUND`` is returned.

The :prop_rule:`COMMAND` rule property can be used as a shorthand to the
``COMMAND_0`` rule property.

The number of commands can be retrieve using the :prop_rule:`COMMAND_COUNT`
rule property.

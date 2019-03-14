variable_watch
--------------

Watch the CMake variable for change.

.. code-block:: cmake

  variable_watch(<variable> [<command>])

If the specified ``<variable>`` changes, a message will be printed
to inform about the change.

Additionally, if ``<command>`` is given, this command will be executed.
The command will receive the following arguments:
``COMMAND(<variable> <access> <value> <current_list_file> <stack>)``

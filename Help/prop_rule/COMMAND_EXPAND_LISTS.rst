COMMAND_EXPAND_LISTS
--------------------

.. versionadded:: 4.5

``COMMAND_EXPAND_LISTS`` is a boolean specifying that the lists in the
``COMMAND`` arguments of the :command:`add_custom_rule` command will be
expanded, including those created with
:manual:`generator expressions <cmake-generator-expressions(7)>`.

By default, ``COMMAND_EXPAND_LISTS`` is true.

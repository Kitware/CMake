RULES
-----

.. versionadded:: 4.5

This read-only directory property contains a
:ref:`semicolon-separated list <CMake Language Lists>` of
rules added in the directory by calls to the :command:`add_custom_rule`
command.
Each entry in the list is the logical name of a rule, suitable
to pass to the :command:`get_property` command ``RULE`` option
when called in the same directory.

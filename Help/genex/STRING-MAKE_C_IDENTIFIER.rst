STRING:MAKE_C_IDENTIFIER
------------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:MAKE_C_IDENTIFIER,string>

.. versionadded:: 4.3

Convert each non-alphanumeric character in the ``string`` to an underscore. If
the first character of the ``string`` is a digit, an underscore will also be
prepended.

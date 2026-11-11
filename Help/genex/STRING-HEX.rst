STRING:HEX
----------

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:HEX,string>

.. versionadded:: 4.3

Convert each byte in the ``string`` to its hexadecimal representation. Letters
in the result (a through f) are in lowercase.

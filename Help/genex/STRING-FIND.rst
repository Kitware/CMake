STRING:FIND
-----------

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:FIND,string[,FROM:(BEGIN|END)],substring>

.. versionadded:: 4.3

The position where the given ``substring`` was found in the supplied ``string``.
If the ``substring`` is not found, a position of -1 is returned.

The ``FROM:`` option defines how the search will be done:

``BEGIN``
  The search will start at the beginning of the ``string``. This the default.

``END``
  The search will start from the end of the ``string``.

The ``$<STRING:FIND>`` generator expression treats all strings as ASCII-only
characters. The index returned will also be counted in bytes, so strings
containing multi-byte characters may lead to unexpected results.

STRING:SUBSTRING
----------------

.. |genex-family| replace::
   :ref:`genex-family-string-queries-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:SUBSTRING,string,begin,length>

.. versionadded:: 4.3

The substring of the given ``string``. If ``length`` is ``-1`` or greater than
the ``string`` length the remainder of the string starting at ``begin`` will be
returned.

Both ``begin`` and ``length`` are counted in bytes, so care must be exercised if
``string`` could contain multi-byte characters.

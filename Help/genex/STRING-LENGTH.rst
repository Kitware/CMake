STRING:LENGTH
-------------

.. genex:: $<STRING:LENGTH,string>
   :target: STRING

.. |genex-family| replace::
   :ref:`genex-family-string-queries-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:LENGTH,string>

.. versionadded:: 4.3

The given string's length in bytes. Note that this means, if ``string`` contains
multi-byte characters, the result will *not* be the number of characters.

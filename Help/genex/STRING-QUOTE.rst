STRING:QUOTE
------------

.. versionadded:: 4.3

.. |genex-family| replace::
   :ref:`genex-family-string-transforming-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<STRING:QUOTE,REGEX,string>

Escape the specified elements of the ``string``. The possible options are:

``REGEX``
  Escape all characters that have special meaning in a regular expressions,
  such that the ``string`` can be used as part of a regular expression to
  match the input literally.

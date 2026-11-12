STRING:STRIP
------------

.. |genex-family| replace::
   :ref:`genex-family-string-transforming-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:STRIP,SPACES,string>

.. versionadded:: 4.3

Remove the specified elements from the ``string``. The possible options are:

``SPACES``
  Remove the leading and trailing spaces of the ``string``.

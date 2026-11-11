STRING:STRIP
------------

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:STRIP,SPACES,string>

.. versionadded:: 4.3

Remove the specified elements from the ``string``. The possible options are:

``SPACES``
  Remove the leading and trailing spaces of the ``string``.

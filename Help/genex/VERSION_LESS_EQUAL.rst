VERSION_LESS_EQUAL
------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<VERSION_LESS_EQUAL:v1,v2>

.. versionadded:: 3.7

``1`` if ``v1`` is a version less than or equal to ``v2``, else ``0``.

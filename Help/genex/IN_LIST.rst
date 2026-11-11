IN_LIST
-------

Syntax
^^^^^^

.. code-block:: cmake

  $<IN_LIST:string,list>

.. versionadded:: 3.12

``1`` if ``string`` is an item in the semicolon-separated ``list``, else ``0``.
It uses case-sensitive comparisons.

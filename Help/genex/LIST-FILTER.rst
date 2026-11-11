LIST:FILTER
-----------

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:FILTER,list,INCLUDE|EXCLUDE,regex>

.. versionadded:: 3.27

A list of items from the ``list`` which match (``INCLUDE``) or do not match
(``EXCLUDE``) the regular expression ``regex``.  The result is the same as
:genex:`$<FILTER:list,INCLUDE|EXCLUDE,regex>`.

.. versionadded:: 4.5

  The regular expression may be introduced explicitly with a ``REGEX``
  keyword, and a ``PREDICATE`` keyword selects items using a generator
  expression instead:

  .. code-block:: cmake

    $<LIST:FILTER,list,INCLUDE|EXCLUDE,REGEX,regex>
    $<LIST:FILTER,list,INCLUDE|EXCLUDE,PREDICATE,body>

  With ``PREDICATE``, ``body`` is evaluated once per item with the bound
  operand :genex:`$<_0>` expanding to the current item.  The body must
  evaluate to exactly ``0`` or ``1``; ``INCLUDE`` keeps items whose body
  yields ``1`` and ``EXCLUDE`` removes them.  Use ``$<BOOL:...>`` to
  coerce other values.  Because ``REGEX`` and ``PREDICATE`` are now keywords,
  a bare regular expression equal to ``REGEX`` or ``PREDICATE`` must use the
  explicit ``REGEX`` form.

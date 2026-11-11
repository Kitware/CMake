LIST:TRANSFORM
--------------

.. genex:: $<LIST:TRANSFORM,list,ACTION[,SELECTOR]>
   :target: LIST

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:TRANSFORM,list,ACTION[,SELECTOR]>

.. versionadded:: 3.27

The ``list`` transformed by applying an ``ACTION`` to all or, by specifying a
``SELECTOR``, to the selected list items.

.. note::

  The ``TRANSFORM`` sub-command does not change the number of items in the
  list. If a ``SELECTOR`` is specified, only some items will be changed,
  the other ones will remain the same as before the transformation.

``ACTION`` specifies the action to apply to the items of the list. The actions
have exactly the same semantics as for the :command:`list(TRANSFORM)` command.
``ACTION`` must be one of the following:

  :command:`APPEND <list(TRANSFORM_APPEND)>`, :command:`PREPEND <list(TRANSFORM_APPEND)>`
    Append, prepend specified value to each item of the list.

    .. code-block:: cmake

      $<LIST:TRANSFORM,list,(APPEND|PREPEND),value[,SELECTOR]>

  :command:`TOLOWER <list(TRANSFORM_TOLOWER)>`, :command:`TOUPPER <list(TRANSFORM_TOLOWER)>`
    Convert each item of the list to lower, upper characters.

    .. code-block:: cmake

      $<LIST:TRANSFORM,list,(TOLOWER|TOUPPER)[,SELECTOR]>

  :command:`STRIP <list(TRANSFORM_STRIP)>`
    Remove leading and trailing spaces from each item of the list.

    .. code-block:: cmake

      $<LIST:TRANSFORM,list,STRIP[,SELECTOR]>

  :command:`REPLACE <list(TRANSFORM_REPLACE)>`:
    Match the regular expression as many times as possible and substitute
    the replacement expression for the match for each item of the list.

    .. code-block:: cmake

      $<LIST:TRANSFORM,list,REPLACE,regular_expression,replace_expression[,SELECTOR]>

    .. versionchanged:: 4.1
      The ``^`` anchor now matches only at the beginning of the input
      element instead of the beginning of each repeated search.
      See policy :policy:`CMP0186`.

  :command:`APPLY <list(TRANSFORM_APPLY)>`
    Transform each selected element by evaluating an arbitrary generator
    expression ``<body>`` once per element.  Within ``<body>``, the bound
    operand :genex:`$<_0>` expands to the current element.  Unlike the
    configure-time :command:`list(TRANSFORM APPLY) <list(TRANSFORM_APPLY)>`
    command, the genex form returns the body's value directly and has no side
    effects.  A list-valued body result expands into multiple elements, like
    any other list-valued generator expression.

    .. code-block:: cmake

      $<LIST:TRANSFORM,list,APPLY,body[,SELECTOR]>

    .. versionadded:: 4.5

``SELECTOR`` determines which items of the list will be transformed. Only one
type of selector can be specified at a time. When given, ``SELECTOR`` must be
one of the following:

  ``AT``
    Specify a list of indexes.

    .. code-block:: cmake

      $<LIST:TRANSFORM,list,ACTION,AT,index[,index...]>

  ``FOR``
    Specify a range with, optionally, an increment used to iterate over the
    range.

    .. code-block:: cmake

      $<LIST:TRANSFORM,list,ACTION,FOR,start,stop[,step]>

  ``REGEX``
    Specify a regular expression.
    Only items matching the regular expression will be transformed.

    .. code-block:: cmake

      $<LIST:TRANSFORM,list,ACTION,REGEX,regular_expression>

  ``PREDICATE``
    Specify a generator expression ``body`` evaluated once per item with the
    bound operand :genex:`$<_0>` expanding to the current item.  Only items
    whose body evaluates to ``1`` are transformed; the body must evaluate to
    exactly ``0`` or ``1``.  ``PREDICATE`` may be combined with any action,
    including ``APPLY`` (in which case both bodies bind :genex:`$<_0>`
    independently).
    Like all selectors, only one selector may be given; ``PREDICATE`` cannot
    be combined with ``AT``, ``FOR``, or ``REGEX``.

    .. code-block:: cmake

      $<LIST:TRANSFORM,list,ACTION,PREDICATE,body>

    .. versionadded:: 4.5

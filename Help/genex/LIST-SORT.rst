LIST:SORT
---------

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:SORT,list[,(COMPARE:option|CASE:option|ORDER:option)]...>

.. versionadded:: 3.27

The ``list`` sorted according to the specified options.

Use one of the ``COMPARE`` options to select the comparison method for sorting:

  ``STRING``
    Sorts a list of strings alphabetically.
    This is the default behavior if the ``COMPARE`` option is not given.

  ``FILE_BASENAME``
    Sorts a list of file paths by their basenames.

  ``NATURAL``
    Sorts a list of strings using natural order (see the man page for
    ``strverscmp(3)``), such that contiguous digits are compared as whole
    numbers.  For example, the following list ``10.0 1.1 2.1 8.0 2.0 3.1``
    will be sorted as ``1.1 2.0 2.1 3.1 8.0 10.0`` if the ``NATURAL``
    comparison is selected, whereas it will be sorted as
    ``1.1 10.0 2.0 2.1 3.1 8.0`` with the ``STRING`` comparison.

Use one of the ``CASE`` options to select a case-sensitive or case-insensitive
sort mode:

  ``SENSITIVE``
    List items are sorted in a case-sensitive manner.
    This is the default behavior if the ``CASE`` option is not given.

  ``INSENSITIVE``
    List items are sorted in a case-insensitive manner.  The order of
    items which differ only by upper/lowercase is not specified.

To control the sort order, one of the ``ORDER`` options can be given:

  ``ASCENDING``
    Sorts the list in ascending order.
    This is the default behavior when the ``ORDER`` option is not given.

  ``DESCENDING``
    Sorts the list in descending order.

Options can be specified in any order, but it is an error to specify the same
option multiple times.

.. code-block:: cmake

  $<LIST:SORT,list,CASE:SENSITIVE,COMPARE:STRING,ORDER:DESCENDING>

.. versionadded:: 4.5

A ``COMPARATOR`` option sorts using a generator-expression ``body`` instead of a
built-in ordering:

.. code-block:: cmake

  $<LIST:SORT,list,COMPARATOR,body[,ORDER:ASCENDING|DESCENDING][,CASE:SENSITIVE|INSENSITIVE]>

``body`` is evaluated once per comparison with the two items being compared
bound to :genex:`$<_0>` and :genex:`$<_1>`.  It must evaluate to exactly ``0``
or ``1``; ``1`` means :genex:`$<_0>` sorts before :genex:`$<_1>`. ``COMPARATOR``
is incompatible with ``COMPARE:``.  ``ORDER:DESCENDING`` reverses the comparator
and ``CASE:INSENSITIVE`` case-folds the values the body sees, both as in the
configure-time :command:`list(SORT)`.  The body must induce a `strict weak
ordering <https://en.wikipedia.org/wiki/Weak_ordering#Strict_weak_orderings>`_.

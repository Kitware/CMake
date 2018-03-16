list
----

.. only:: html

   .. contents::

List operations.

The list subcommands ``APPEND``, ``INSERT``, ``FILTER``, ``REMOVE_AT``,
``REMOVE_ITEM``, ``REMOVE_DUPLICATES``, ``REVERSE`` and ``SORT`` may create
new values for the list within the current CMake variable scope.  Similar to
the :command:`set` command, the LIST command creates new variable values in
the current scope, even if the list itself is actually defined in a parent
scope.  To propagate the results of these operations upwards, use
:command:`set` with ``PARENT_SCOPE``, :command:`set` with
``CACHE INTERNAL``, or some other means of value propagation.

.. note::

  A list in cmake is a ``;`` separated group of strings.  To create a
  list the set command can be used.  For example, ``set(var a b c d e)``
  creates a list with ``a;b;c;d;e``, and ``set(var "a b c d e")`` creates a
  string or a list with one item in it.   (Note macro arguments are not
  variables, and therefore cannot be used in LIST commands.)

.. note::

  When specifying index values, if ``<element index>`` is 0 or greater, it
  is indexed from the beginning of the list, with 0 representing the
  first list element.  If ``<element index>`` is -1 or lesser, it is indexed
  from the end of the list, with -1 representing the last list element.
  Be careful when counting with negative indices: they do not start from
  0.  -0 is equivalent to 0, the first list element.

Capacity and Element access
^^^^^^^^^^^^^^^^^^^^^^^^^^^

LENGTH
""""""

::

  list(LENGTH <list> <output variable>)

Returns the list's length.

GET
"""

::

  list(GET <list> <element index> [<element index> ...] <output variable>)

Returns the list of elements specified by indices from the list.

Search
^^^^^^

FIND
""""

::

  list(FIND <list> <value> <output variable>)

Returns the index of the element specified in the list or -1
if it wasn't found.

Modification
^^^^^^^^^^^^

APPEND
""""""

::

  list(APPEND <list> [<element> ...])

Appends elements to the list.

FILTER
""""""

::

  list(FILTER <list> <INCLUDE|EXCLUDE> REGEX <regular_expression>)

Includes or removes items from the list that match the mode's pattern.
In ``REGEX`` mode, items will be matched against the given regular expression.

For more information on regular expressions see also the
:command:`string` command.

INSERT
""""""

::

  list(INSERT <list> <element_index> <element> [<element> ...])

Inserts elements to the list to the specified location.

REMOVE_ITEM
"""""""""""

::

  list(REMOVE_ITEM <list> <value> [<value> ...])

Removes the given items from the list.

REMOVE_AT
"""""""""

::

  list(REMOVE_AT <list> <index> [<index> ...])

Removes items at given indices from the list.

REMOVE_DUPLICATES
"""""""""""""""""

::

  list(REMOVE_DUPLICATES <list>)

Removes duplicated items in the list.

Sorting
^^^^^^^

REVERSE
"""""""

::

  list(REVERSE <list>)

Reverses the contents of the list in-place.

SORT
""""

::

  list(SORT <list>)


Sorts the list in-place alphabetically.

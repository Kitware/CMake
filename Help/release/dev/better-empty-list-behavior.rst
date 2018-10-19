better-empty-list-behavior
--------------------------

* The :command:`list` operations ``REMOVE_ITEM``, ``REMOVE_DUPLICATES``,
  ``SORT``, ``REVERSE``, and ``FILTER`` all now accept a non-existent variable
  as the list since these operations on empty lists is also the empty list.

* The :command:`list` operation ``REMOVE_AT`` now indicates that the given
  indices are invalid for a non-existent variable or empty list.

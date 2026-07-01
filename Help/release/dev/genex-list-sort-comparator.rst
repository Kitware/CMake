genex-list-sort-comparator
--------------------------

* The :genex:`LIST` generator expression's ``SORT`` operation gained a
  ``COMPARATOR`` option that orders items using an arbitrary generator
  expression evaluated once per comparison, with ``$<_0>`` and ``$<_1>``
  referring to the two items being compared.

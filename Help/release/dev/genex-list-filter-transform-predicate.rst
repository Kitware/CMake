genex-list-filter-transform-predicate
-------------------------------------

* The :genex:`LIST` generator expression's ``TRANSFORM`` operation gained a
  ``PREDICATE`` selector that chooses the items to transform by evaluating an
  arbitrary generator expression once per item, with ``$<_0>`` referring to the
  current item.

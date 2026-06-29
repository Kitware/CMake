genex-list-filter-transform-predicate
-------------------------------------

* The :genex:`LIST` generator expression's ``FILTER`` and ``TRANSFORM``
  operations gained a ``PREDICATE`` keyword that selects items by evaluating an
  arbitrary generator expression once per item, with ``$<_0>`` referring to the
  current item.  ``FILTER`` also gained an explicit ``REGEX`` keyword.

list-PREDICATE
--------------

* The :command:`list(TRANSFORM)` command gained a new ``PREDICATE`` selector
  that invokes a user-defined :command:`function` to decide which elements are
  transformed.

* The :command:`list(FILTER)` command gained a new ``PREDICATE`` mode
  that invokes a user-defined :command:`function` to decide which elements are
  included or excluded, complementing the existing ``REGEX`` mode.

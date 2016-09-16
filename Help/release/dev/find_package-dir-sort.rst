find_package-dir-sort
---------------------

* The :command:`find_package` command gained the possibility of
  sorting compatible libraries by ``NAME`` or by ``NATURAL`` sorting by
  setting the two new variables :variable:`CMAKE_FIND_PACKAGE_SORT_ORDER`
  and :variable:`CMAKE_FIND_PACKAGE_SORT_DIRECTION`.

* Variable :variable:`CMAKE_FIND_PACKAGE_SORT_ORDER` was added to control
  the sorting mode of the :command:`find_package` command.

* Variable :variable:`CMAKE_FIND_PACKAGE_SORT_DIRECTION` was added to control
  the sorting direction the :command:`find_package` command.

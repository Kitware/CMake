CMAKE_FIND_PACKAGE_SORT_ORDER
-----------------------------

.. versionadded:: 3.7

The default order for sorting directories which match a search path containing
a glob expression found using :command:`find_package`.  It can assume one of
the following values:

``NONE``
  Default.  No attempt is done to sort directories.
  The first valid package found will be selected.

``NAME``
  Sort directories lexicographically before searching.

``NATURAL``
  Sort directories using natural order (see ``strverscmp(3)`` manual),
  i.e. such that contiguous digits are compared as whole numbers.

Natural sorting can be employed to return the highest version when multiple
versions of the same library are available to be found by
:command:`find_package`.  For example suppose that the following libraries
have package configuration files on disk, in a directory of the same name,
with all such directories residing in the same parent directory:

* libX-1.1.0
* libX-1.2.9
* libX-1.2.10

By setting ``NATURAL`` order we can select the one with the highest
version number ``libX-1.2.10``.

.. code-block:: cmake

  set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
  find_package(libX CONFIG)

The sort direction can be controlled using the
:variable:`CMAKE_FIND_PACKAGE_SORT_DIRECTION` variable
(by default descending, e.g. lib-B will be tested before lib-A).

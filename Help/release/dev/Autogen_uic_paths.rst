AutoGen uic paths
-----------------

* Variable :variable:`CMAKE_AUTOUIC_SEARCH_PATHS` was introduced to
  allow :variable:`CMAKE_AUTOUIC` to search for ``foo.ui`` in more
  places than the vicinity of the ``ui_foo.h`` including file.

* The new target property :prop_tgt:`AUTOUIC_SEARCH_PATHS` was introduced to
  allow :prop_tgt:`AUTOUIC` to search for ``foo.ui`` in more
  places than the vicinity of the ``ui_foo.h`` including file.

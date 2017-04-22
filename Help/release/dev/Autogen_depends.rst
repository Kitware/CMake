AutoGen depends
---------------

* Variable :variable:`CMAKE_AUTOMOC_DEPEND_FILTERS` was introduced to
  allow :variable:`CMAKE_AUTOMOC` to extract additional dependency file names
  for ``moc`` from the contents of source files.

* The new target property :prop_tgt:`AUTOMOC_DEPEND_FILTERS` was introduced to
  allow :prop_tgt:`AUTOMOC` to extract additional dependency file names
  for ``moc`` from the contents of source files.

install-nonextant-file-set
--------------------------

* The :command:`install(TARGETS)` command no longer ignores file sets which
  haven't been defined at the point it is called. The ordering of
  :command:`target_sources(FILE_SET)` and ``install(TARGETS)`` is no longer
  semantically relevant.

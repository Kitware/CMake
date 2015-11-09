FindGTK2_GTK2_TARGETS
---------------------

* The :module:`FindGTK2` module, when ``GTK2_USE_IMPORTED_TARGETS`` is
  enabled, now sets ``GTK2_LIBRARIES`` to contain the list of imported
  targets instead of the paths to the libraries.  Moreover it now sets
  a new ``GTK2_TARGETS`` variable containing all the targets imported.

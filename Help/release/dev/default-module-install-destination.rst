default-module-install-destination
----------------------------------

* The :command:`install(TARGETS)` command no longer requires a
  ``LIBRARY DESTINATION`` for :ref:`Module Libraries`.  When no destination is
  given, a module library is installed to the default ``RUNTIME``
  destination on DLL platforms and to the default ``LIBRARY`` destination
  on other platforms.

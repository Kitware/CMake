Find Modules
------------

* The uppercased ``<PACKAGENAME>_FOUND`` result variables of find modules
  are now deprecated in favor of ``<PackageName>_FOUND`` result variables,
  where appropriate.  See documentation of each find module for details.

* Nearly all find modules now provide a ``<PackageName>_VERSION`` result
  variable matching the casing of its module name.  Existing variants such as
  ``<PackageName>_VERSION_STRING`` and uppercased ``<PACKAGENAME>_VERSION``
  are deprecated.  See documentation of each find module for details.

* The :module:`FindwxWidgets` module's result variable
  ``wxWidgets_USE_FILE`` is now deprecated in favor of including the
  :module:`UsewxWidgets` module directly.

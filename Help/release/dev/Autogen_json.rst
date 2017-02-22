AutoGen json
------------

* When using :prop_tgt:`AUTOMOC`, CMake scans for the presence of the
  ``Q_PLUGIN_METADATA`` macro and reruns moc when the file from the
  macro's ``FILE`` argument changes.

* When :prop_tgt:`AUTOMOC` detects an include statement of the form
  ``#include "moc_<basename>.cpp"`` the respective header file is searched
  for in the :prop_tgt:`INCLUDE_DIRECTORIES` of the target as well.

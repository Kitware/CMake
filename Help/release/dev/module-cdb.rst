module-cdb
==========

* Targets with C++ modules may now export their module compile commands using
  the :prop_tgt:`EXPORT_BUILD_DATABASE` target property. This is initialized
  with the :variable:`CMAKE_EXPORT_BUILD_DATABASE` variable which is itself
  initialized using the :envvar:`CMAKE_EXPORT_BUILD_DATABASE` environment
  variable. Only supported with the :ref:`Ninja Generators`.

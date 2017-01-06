QtAutogen_Contain
-----------------

* When using AUTOMOC or AUTOUIC, generated
  ``moc_*``, ``*.moc`` and ``ui_*`` are placed in the
  ``<CMAKE_CURRENT_BINARY_DIR>/<TARGETNAME>_autogen/include`` directory which
  is automatically added to the target's :prop_tgt:`INCLUDE_DIRECTORIES`.
  It is therefore not necessary anymore to have
  :variable:`CMAKE_CURRENT_BINARY_DIR` in the target's
  :prop_tgt:`INCLUDE_DIRECTORIES`.

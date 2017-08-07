autogen-configs
---------------

* When using :prop_tgt:`AUTOMOC` or :prop_tgt:`AUTOUIC` with a
  multi configuration generator (e.g. :generator:`Xcode`),
  included ``*.moc``,  ``moc_*.cpp`` and ``ui_*.h`` files are generated in
  ``<AUTOGEN_BUILD_DIR>/include_<CONFIG>`` instead of
  ``<AUTOGEN_BUILD_DIR>/include``.

automoc-cxx-modules-scan
------------------------

* :prop_tgt:`AUTOMOC` now has its generated ``mocs_compilation.cpp``
  scanned for C++ module dependencies under the same conditions as the
  target's own sources.  A header that imports a module may now be
  processed by ``moc``.

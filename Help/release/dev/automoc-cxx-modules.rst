automoc-cxx-modules
-------------------

* :prop_tgt:`AUTOMOC` now processes C++ module interface and partition
  units that are members of a ``FILE_SET`` of type ``CXX_MODULES``.
  ``moc`` is run on such units and its generated output is compiled as a
  module implementation unit of the same module.  This requires Qt 6.13
  or newer, whose ``moc`` supports C++ module units.

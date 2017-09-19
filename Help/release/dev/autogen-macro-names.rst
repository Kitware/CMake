autogen-macro-names
-------------------

* When using :prop_tgt:`AUTOMOC`, CMake searches for the strings ``Q_OBJECT``
  and ``Q_OBJECT`` in a source file to determine if it needs to be ``moc``
  processed. The new variable :variable:`CMAKE_AUTOMOC_MACRO_NAMES` allows to
  register additional strings (macro names) so search for.

* When using :prop_tgt:`AUTOMOC`, CMake searches for the strings ``Q_OBJECT``
  and ``Q_OBJECT`` in a source file to determine if it needs to be ``moc``
  processed. The new target property :prop_tgt:`AUTOMOC_MACRO_NAMES` allows to
  register additional strings (macro names) so search for.

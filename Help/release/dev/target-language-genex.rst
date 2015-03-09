target-language-genex
---------------------

* A new ``COMPILE_LANGUAGE`` generator expression was introduced to
  allow specification of compile options for target files based on the
  :prop_sf:`LANGUAGE` of each source file.  Due to limitations of the
  underlying native build tools, this feature has varying support across
  generators.  See the :manual:`cmake-generator-expressions(7)` manual
  for details.

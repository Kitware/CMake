install-name-dir-genex
----------------------

* The :prop_tgt:`INSTALL_NAME_DIR` target property now supports
  :manual:`generator expressions <cmake-generator-expressions(7)>`.
  In particular, the ``$<INSTALL_PREFIX>`` generator expression can
  be used to set the directory relative to the install-time prefix.

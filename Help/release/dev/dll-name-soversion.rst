dll-name-soversion
------------------

* The :variable:`CMAKE_DLL_NAME_WITH_SOVERSION` variable and associated
  :prop_tgt:`DLL_NAME_WITH_SOVERSION` target property were added to
  optionally append the :prop_tgt:`SOVERSION` to the filename of the
  ``.dll`` part of a shared library on Windows.

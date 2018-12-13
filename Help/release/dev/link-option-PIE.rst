link-option-PIE
---------------

* Required link options to manage Position Independent Executable are now
  added when :prop_tgt:`POSITION_INDEPENDENT_CODE` is set.  The project is
  responsible for using the :module:`CheckPIESupported` module to check for
  ``PIE`` support to ensure that the :prop_tgt:`POSITION_INDEPENDENT_CODE`
  target property will be honored at link time for executables.  This behavior
  is controlled by policy :policy:`CMP0083`.

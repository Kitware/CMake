cpack-deb-source-date-epoch
---------------------------

* The :cpack_gen:`CPack Deb Generator` learned to honor the ``SOURCE_DATE_EPOCH``
  environment variable when packaging files.  This is useful for generating
  reproducible packages.

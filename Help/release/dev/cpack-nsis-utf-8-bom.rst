cpack-nsis-utf8-bom
-------------------

* The :cpack_gen:`CPack NSIS Generator` now handles correctly Unicode characters.
  If you want to have a ``CPACK_RESOURCE_FILE_LICENSE`` with UTF-8 characters
  it needs to be encoded in UTF-8 BOM.

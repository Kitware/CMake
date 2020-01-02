cpack-nsis-headerimage_var
--------------------------

* The :cpack_gen:`CPack NSIS Generator` gained a new variable
  :variable:`CPACK_NSIS_MUI_HEADERIMAGE` to set the header image.
  To not break existing setups, it still defaults to
  :variable:`CPACK_PACKAGE_ICON` if the new variable is not set.

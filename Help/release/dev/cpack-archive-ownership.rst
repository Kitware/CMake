cpack-archive-ownership
-----------------------

* The :cpack_gen:`CPack Archive Generator` generator gained new
  :variable:`CPACK_ARCHIVE_UID` and :variable:`CPACK_ARCHIVE_GID`
  options to specify the UID and GID of archive entries.
  The default now UID 0 and GID 0.  See policy :policy:`CMP0206`.

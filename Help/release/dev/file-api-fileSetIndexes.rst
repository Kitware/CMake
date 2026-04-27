file-api-fileSetIndexes
-----------------------

* The :manual:`cmake-file-api(7)` "codemodel" version 2 "target" "sources" and
  "interfaceSources" objects gained new ``fileSetIndexes`` fields to support
  sources belonging to multiple file sets. The ``fileSetIndex`` fields are
  maintained for backwards compatibility only, and users are advised to port
  to the new fields.

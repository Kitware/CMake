project-vars-policy
-------------------

* The :command:`project` command now always sets
  :variable:`<PROJECT-NAME>_SOURCE_DIR`, :variable:`<PROJECT-NAME>_BINARY_DIR`,
  and :variable:`<PROJECT-NAME>_IS_TOP_LEVEL` as both normal variables and
  cache entries.  See policy :policy:`CMP0180`.

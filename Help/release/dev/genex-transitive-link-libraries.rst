genex-transitive-link-libraries
-------------------------------

* The :genex:`TARGET_PROPERTY` generator expression now evaluates the
  :prop_tgt:`LINK_LIBRARIES` and :prop_tgt:`INTERFACE_LINK_LIBRARIES`
  target properties transitively.  See policy :policy:`CMP0189`.

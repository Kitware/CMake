genex-link-properties
---------------------

* The :genex:`TARGET_PROPERTY` generator expression now evaluates target
  properties :prop_tgt:`INTERFACE_LINK_OPTIONS`,
  :prop_tgt:`INTERFACE_LINK_DIRECTORIES`, and
  :prop_tgt:`INTERFACE_LINK_DEPENDS` correctly by following private
  dependencies of static libraries.  See policy :policy:`CMP0166`.

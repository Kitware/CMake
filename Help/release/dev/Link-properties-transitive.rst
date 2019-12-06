Link-properties-transitive
--------------------------

* Target link properties :prop_tgt:`INTERFACE_LINK_OPTIONS`,
  :prop_tgt:`INTERFACE_LINK_DIRECTORIES` and
  :prop_tgt:`INTERFACE_LINK_DEPENDS` are now transitive over private
  dependency on static libraries.
  See policy :policy:`CMP0099`.

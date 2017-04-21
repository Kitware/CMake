wix-attributes-patch
--------------------

* The patching system within the :module:`CPackWIX` module now allows the
  ability to set additional attributes.  This can be done by specifying
  addional attributes with the ``CPackWiXFragment`` XML tag after the
  ``Id`` attribute.  See the :variable:`CPACK_WIX_PATCH_FILE` variable.

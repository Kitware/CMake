wix-multi-cab
-------------

* :variable:`CPACK_WIX_CAB_PER_COMPONENT` allows CPack WIX opt-in generation of one
  `.cab` file per component. Having multiple `.cab` files may improve the time it takes
  to generate installers and may also work around per `.cab` size constraints.

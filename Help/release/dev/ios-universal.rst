ios-universal
-------------

* When building for embedded Apple platforms like iOS CMake learned to build and
  install combined targets which contain both a device and a simulator build.
  This behavior can be enabled by setting the :prop_tgt:`IOS_INSTALL_COMBINED`
  target property.

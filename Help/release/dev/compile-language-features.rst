target-language-features
------------------------

* New :prop_tgt:`CXX_STANDARD` and :prop_tgt:`CXX_EXTENSIONS` target
  properties may specify values which CMake uses to compute required
  compile options such as ``-std=c++11`` or ``-std=gnu++11``. The
  :variable:`CMAKE_CXX_STANDARD` and :variable:`CMAKE_CXX_EXTENSIONS`
  variables may be set to initialize the target properties.

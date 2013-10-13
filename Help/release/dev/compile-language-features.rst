target-language-features
------------------------

* New :prop_tgt:`CXX_STANDARD` and :prop_tgt:`CXX_EXTENSIONS` target
  properties may specify values which CMake uses to compute required
  compile options such as ``-std=c++11`` or ``-std=gnu++11``. The
  :variable:`CMAKE_CXX_STANDARD` and :variable:`CMAKE_CXX_EXTENSIONS`
  variables may be set to initialize the target properties.

* New :prop_tgt:`COMPILE_FEATURES` target property may contain a list
  of features required to compile a target.  CMake uses this
  information to ensure that the compiler in use is capable of building
  the target, and to add any necessary compile flags to support language
  features.

* New :command:`target_compile_features` command allows populating the
  :prop_tgt:`COMPILE_FEATURES` target property, just like any other
  build variable.

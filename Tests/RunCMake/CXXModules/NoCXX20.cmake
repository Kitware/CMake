enable_language(CXX)

add_library(nocxx20)
target_sources(nocxx20
  PUBLIC
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/module.cxx)
set_target_properties(nocxx20
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON)

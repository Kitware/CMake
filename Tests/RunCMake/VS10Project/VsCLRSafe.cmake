enable_language(CXX)

add_library(foo foo.cpp)

set_target_properties(foo PROPERTIES
  COMMON_LANGUAGE_RUNTIME "safe")

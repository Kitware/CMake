enable_language(CXX)

add_library(nocxx20)
target_sources(nocxx20
  PUBLIC
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/module.cxx)
target_compile_features(nocxx20
  PRIVATE
    cxx_std_17)

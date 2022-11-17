enable_language(C)

add_library(nocxx)
target_sources(nocxx
  PRIVATE
    sources/c-anchor.c
  PUBLIC
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/module.cxx)

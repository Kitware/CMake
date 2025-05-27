set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

enable_language(CXX)

add_library(install-bmi-generic-args)
target_sources(install-bmi-generic-args
  PRIVATE
    sources/cxx-anchor.cxx)

install(TARGETS install-bmi-generic-args
  DESTINATION "bin")

enable_language(CXX)

add_library(install-bmi-generic-args)
target_sources(install-bmi-generic-args
  PRIVATE
    sources/cxx-anchor.cxx)

install(TARGETS install-bmi-generic-args
  DESTINATION "bin")

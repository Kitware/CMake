enable_language(CXX)
set(CMAKE_CXX_SCANDEP_SOURCE "echo")

# TODO(cxxmodules): Add instances of this test which test the policy
# of the property's unset behavior.
# set(CMAKE_CXX_MODULE_STD …)

add_library(nocxx23target)
target_sources(nocxx23target
  PRIVATE
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/module.cxx)
target_compile_features(nocxx23target PRIVATE cxx_std_23)

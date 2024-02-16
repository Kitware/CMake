set(CMAKE_CONFIGURATION_TYPES Debug Release)
enable_language(CXX)

# Test several generator code paths covering different target types.
add_library(default empty.cxx)
add_library(defaultCLR empty.cxx)
set_property(TARGET defaultCLR PROPERTY COMMON_LANGUAGE_RUNTIME "")
add_library(defaultRTL empty.cxx)
set_property(TARGET defaultRTL PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
add_custom_target(defaultUtil)

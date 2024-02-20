set(CMAKE_CONFIGURATION_TYPES Debug Release)
enable_language(CXX)

# An empty string suppresses generation of the setting.
set(CMAKE_VS_USE_DEBUG_LIBRARIES "")
add_library(empty empty.cxx)
add_library(emptyCLR empty.cxx)
set_property(TARGET emptyCLR PROPERTY COMMON_LANGUAGE_RUNTIME "")
add_custom_target(emptyUtil)

# A generator expression can encode per-config values.
set(CMAKE_VS_USE_DEBUG_LIBRARIES "$<CONFIG:Debug>")
add_library(genex empty.cxx)
add_library(genexCLR empty.cxx)
set_property(TARGET genexCLR PROPERTY COMMON_LANGUAGE_RUNTIME "")
add_custom_target(genexUtil)

# The last setting in the top-level directcory affects
# the builtin targets like ALL_BUILD and ZERO_CHECK.
set(CMAKE_VS_USE_DEBUG_LIBRARIES 0)

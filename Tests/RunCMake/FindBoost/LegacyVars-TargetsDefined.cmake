# Use a modern style BoostConfig that defines all targets
# as Boost::<component> with appropriate properties set
# No legacy variables defined in the BoostConfig
set(Boost_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMakePackage_New)

include(${CMAKE_CURRENT_SOURCE_DIR}/LegacyVars.cmake)

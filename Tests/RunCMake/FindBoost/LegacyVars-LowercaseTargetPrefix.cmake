# Use a (made up) modern style BoostConfig that defines all targets
# as boost::<component> with appropriate properties set
# No legacy variables defined in the BoostConfig
set(Boost_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMakePackage_LowerCaseTargetPrefix)

include(${CMAKE_CURRENT_SOURCE_DIR}/LegacyVars.cmake)

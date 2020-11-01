# Use a (made up?) BoostConfig that defines all targets
# as Boost::<component> with appropriate properties set
# But no Boost::headers or Boost::boost target is defined
set(Boost_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMakePackage_NoHeaderTarget)

include(${CMAKE_CURRENT_SOURCE_DIR}/LegacyVars.cmake)

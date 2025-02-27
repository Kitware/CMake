project(DependsMultipleDifferentSets CXX)
include(DependsMultipleCommon.cmake)

install(TARGETS foo EXPORT foo-alt)
install(EXPORT foo-alt DESTINATION cmake)

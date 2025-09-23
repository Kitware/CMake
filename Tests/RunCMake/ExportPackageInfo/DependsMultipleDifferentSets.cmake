project(DependsMultipleDifferentSets CXX)
include(DependsMultipleCommon.cmake)

install(TARGETS foo EXPORT foo-alt)
export(EXPORT foo-alt)

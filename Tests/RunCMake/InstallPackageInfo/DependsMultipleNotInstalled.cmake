project(DependsMultipleNotInstalled CXX)
set(NAMESPACE foo::)
include(DependsMultipleCommon.cmake)
install(TARGETS foo EXPORT foo-alt) # set foo-alt never installed

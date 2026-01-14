add_library(foo foo.cxx)
add_library(bar foo.cxx)
target_link_libraries(bar foo)

install(TARGETS foo EXPORT foo)
export(EXPORT foo NAMESPACE "${NAMESPACE}")
export(PACKAGE_INFO foo EXPORT foo)

install(TARGETS bar EXPORT bar)
export(EXPORT bar)
export(PACKAGE_INFO bar EXPORT bar)

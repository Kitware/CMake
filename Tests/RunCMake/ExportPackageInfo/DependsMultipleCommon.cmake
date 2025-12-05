add_library(foo foo.cxx)
add_library(bar foo.cxx)
target_link_libraries(bar foo)

install(TARGETS foo EXPORT foo)
export(EXPORT foo NAMESPACE "${NAMESPACE}")
export(EXPORT foo PACKAGE_INFO foo)

install(TARGETS bar EXPORT bar)
export(EXPORT bar)
export(EXPORT bar PACKAGE_INFO bar)

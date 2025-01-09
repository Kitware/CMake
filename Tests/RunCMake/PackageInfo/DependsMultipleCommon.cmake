add_library(foo foo.cxx)
add_library(bar foo.cxx)
target_link_libraries(bar foo)

install(TARGETS foo EXPORT foo)
install(EXPORT foo DESTINATION cmake NAMESPACE ${NAMESPACE})
install(PACKAGE_INFO foo EXPORT foo DESTINATION cps)

install(TARGETS bar EXPORT bar)
install(EXPORT bar DESTINATION .)
install(PACKAGE_INFO bar EXPORT bar DESTINATION cps)

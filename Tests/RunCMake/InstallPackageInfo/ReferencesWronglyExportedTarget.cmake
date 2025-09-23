add_library(foo INTERFACE)
add_library(bar INTERFACE)

add_library(test INTERFACE)
target_link_libraries(test INTERFACE foo bar)

install(TARGETS foo EXPORT foo DESTINATION .)
install(TARGETS bar EXPORT bar DESTINATION .)

install(EXPORT foo DESTINATION .)
install(EXPORT bar DESTINATION . NAMESPACE bar_)

install(TARGETS test EXPORT test DESTINATION .)
install(PACKAGE_INFO test EXPORT test)

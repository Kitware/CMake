project(LinkOnly CXX)

add_library(linkOnlyOne foo.cxx)
add_library(linkOnlyTwo foo.cxx)
add_library(foo foo.cxx)
add_library(bar foo.cxx)
target_link_libraries(bar $<LINK_ONLY:linkOnlyOne> $<LINK_ONLY:linkOnlyTwo> foo)

install(TARGETS foo linkOnlyOne linkOnlyTwo EXPORT foo)
export(EXPORT foo PACKAGE_INFO foo)

install(TARGETS bar EXPORT bar)
export(EXPORT bar PACKAGE_INFO bar)

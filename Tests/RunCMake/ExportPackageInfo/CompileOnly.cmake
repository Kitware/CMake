project(LinkOnly CXX)

add_library(linkOnlyOne foo.cxx)
add_library(linkOnlyTwo foo.cxx)
add_library(foo foo.cxx)
add_library(bar foo.cxx)
target_link_libraries(bar $<COMPILE_ONLY:linkOnlyOne> $<COMPILE_ONLY:linkOnlyTwo> foo)

install(TARGETS foo linkOnlyOne linkOnlyTwo EXPORT foo)
export(PACKAGE_INFO foo EXPORT foo)

install(TARGETS bar EXPORT bar)
export(PACKAGE_INFO bar EXPORT bar)

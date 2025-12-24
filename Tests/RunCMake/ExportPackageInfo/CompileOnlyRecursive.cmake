project(LinkOnly CXX)

add_library(foo foo.cxx)
add_library(bar foo.cxx)
target_link_libraries(bar $<COMPILE_ONLY:$<COMPILE_ONLY:foo>>)

install(TARGETS foo EXPORT foo)
export(EXPORT foo PACKAGE_INFO foo)

install(TARGETS bar EXPORT bar)
export(EXPORT bar PACKAGE_INFO bar)

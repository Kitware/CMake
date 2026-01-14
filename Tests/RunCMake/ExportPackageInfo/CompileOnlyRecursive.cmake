project(LinkOnly CXX)

add_library(foo foo.cxx)
add_library(bar foo.cxx)
target_link_libraries(bar $<COMPILE_ONLY:$<COMPILE_ONLY:foo>>)

install(TARGETS foo EXPORT foo)
export(PACKAGE_INFO foo EXPORT foo)

install(TARGETS bar EXPORT bar)
export(PACKAGE_INFO bar EXPORT bar)

project(LinkInterfaceGeneratorExpression CXX)

add_library(foo foo.cxx)
add_library(bar foo.cxx)
target_link_libraries(bar $<1:foo> $<1:$<CONFIG>>)

install(TARGETS foo EXPORT foo)
export(PACKAGE_INFO foo EXPORT foo)

install(TARGETS bar EXPORT bar)
export(PACKAGE_INFO bar EXPORT bar)

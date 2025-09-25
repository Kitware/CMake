add_library(mammal INTERFACE)
add_library(canine INTERFACE)
target_link_libraries(canine INTERFACE mammal)

install(TARGETS mammal EXPORT mammal DESTINATION .)
install(TARGETS canine EXPORT canine DESTINATION .)

export(EXPORT mammal PACKAGE_INFO foo VERSION 1.0)
export(EXPORT canine PACKAGE_INFO foo APPENDIX dog)

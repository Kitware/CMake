add_library(mammal INTERFACE)
add_library(canine INTERFACE)
target_link_libraries(canine INTERFACE mammal)

install(TARGETS mammal EXPORT mammal DESTINATION .)
install(TARGETS canine EXPORT canine DESTINATION .)

install(PACKAGE_INFO foo DESTINATION cps EXPORT mammal VERSION 1.0)
install(PACKAGE_INFO foo DESTINATION cps EXPORT canine APPENDIX dog)

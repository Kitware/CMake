add_library(mammal INTERFACE)
add_library(canine INTERFACE)
target_link_libraries(canine INTERFACE mammal)

install(TARGETS canine EXPORT dog DESTINATION .)
install(PACKAGE_INFO dog EXPORT dog)

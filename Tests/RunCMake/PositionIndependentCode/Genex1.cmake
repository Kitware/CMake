
add_library(genex_pic UNKNOWN IMPORTED)
# PIC is ON if sibling target is a library, OFF if it is an executable
set_property(TARGET genex_pic PROPERTY INTERFACE_POSITION_INDEPENDENT_CODE $<NOT:$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>>)


add_library(conflict1 STATIC main.cpp)
set_property(TARGET conflict1 PROPERTY POSITION_INDEPENDENT_CODE OFF)
target_link_libraries(conflict1 PRIVATE genex_pic)

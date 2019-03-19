
add_library(genex_pic UNKNOWN IMPORTED)
# PIC is ON if sibling target is a library, OFF if it is an executable
set_property(TARGET genex_pic PROPERTY INTERFACE_POSITION_INDEPENDENT_CODE $<NOT:$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>>)


add_executable(conflict2 main.cpp)
set_property(TARGET conflict2 PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(conflict2 PRIVATE genex_pic)

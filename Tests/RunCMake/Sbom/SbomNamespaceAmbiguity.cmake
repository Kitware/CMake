include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)

add_library(libb INTERFACE)
add_library(libc INTERFACE)

target_link_libraries(libc INTERFACE libb)

install(TARGETS libb EXPORT foo DESTINATION .)
install(TARGETS libc EXPORT bar DESTINATION .)

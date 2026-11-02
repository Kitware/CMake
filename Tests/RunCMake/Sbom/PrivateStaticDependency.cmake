include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)

find_package(bar REQUIRED CONFIG
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
)

add_library(foo STATIC ${CMAKE_CURRENT_LIST_DIR}/test.c)
add_library(sibling STATIC ${CMAKE_CURRENT_LIST_DIR}/test.c)
target_link_libraries(foo PRIVATE sibling $<LINK_ONLY:bar::bar>)

install(TARGETS foo sibling EXPORT foo DESTINATION .)

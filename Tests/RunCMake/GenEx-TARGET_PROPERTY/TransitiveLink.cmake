enable_language(C)

add_library(foo1 STATIC empty.c)
target_link_libraries(foo1 PRIVATE foo2 foo3)
target_link_directories(foo1 INTERFACE dir1)
target_link_options(foo1 INTERFACE -opt1)
set_target_properties(foo1 PROPERTIES
  INTERFACE_LINK_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/dep1"
  )
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/dep1" "")

add_library(foo2 STATIC empty.c)
target_link_directories(foo2 INTERFACE dir2)
target_link_options(foo2 INTERFACE -opt2)
set_target_properties(foo2 PROPERTIES
  INTERFACE_LINK_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/dep2"
  )
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/dep2" "")

add_library(foo3 STATIC empty.c)
target_link_directories(foo3 PRIVATE dir3)
target_link_options(foo3 PRIVATE -opt3)
set_target_properties(foo3 PROPERTIES
  LINK_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/dep3"
  )
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/dep3" "")

add_executable(main main.c)
target_link_libraries(main PRIVATE foo1)
target_link_directories(main PRIVATE dirM)
target_link_options(main PRIVATE -optM)
set_target_properties(main PROPERTIES
  LINK_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/depM"
  )
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/depM" "")

file(GENERATE OUTPUT out.txt CONTENT "# file(GENERATE) produced:
main LINK_LIBRARIES: '$<TARGET_PROPERTY:main,LINK_LIBRARIES>' # not transitive
main LINK_DIRECTORIES: '$<TARGET_PROPERTY:main,LINK_DIRECTORIES>'
main LINK_OPTIONS: '$<TARGET_PROPERTY:main,LINK_OPTIONS>'
main LINK_DEPENDS: '$<TARGET_PROPERTY:main,LINK_DEPENDS>'
")

enable_language(C)

add_executable(exe main.c)
add_library(lib1 SHARED obj1.c)
set_property(TARGET lib1 PROPERTY PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/obj3.h)
add_library(lib2 STATIC obj3.c)
add_library(lib3 SHARED obj4.c)
set_property(TARGET lib3 PROPERTY PRIVATE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/obj4.h)
add_library(lib4 SHARED obj5.c)
set_property(TARGET lib4 PROPERTY PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/obj5.h)

add_library(iface INTERFACE)
set_target_properties(iface PROPERTIES
  PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/obj1.h
  PRIVATE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/obj2.h)

install(TARGETS exe lib1 lib2)
install(TARGETS lib3
  LIBRARY DESTINATION lib3
  ARCHIVE DESTINATION lib3
  )
install(TARGETS lib4
  LIBRARY DESTINATION lib4
  RUNTIME DESTINATION lib4
  )
install(TARGETS iface
  PUBLIC_HEADER DESTINATION include
  PRIVATE_HEADER DESTINATION include)

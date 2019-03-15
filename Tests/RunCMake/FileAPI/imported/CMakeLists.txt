project(Imported)

add_library(imported_lib UNKNOWN IMPORTED)
add_executable(imported_exe IMPORTED)
add_executable(link_imported_exe ../empty.c)
target_link_libraries(link_imported_exe PRIVATE imported_lib)

add_library(imported_shared_lib SHARED IMPORTED)
add_executable(link_imported_shared_exe ../empty.c)
target_link_libraries(link_imported_shared_exe PRIVATE imported_shared_lib)

add_library(imported_static_lib STATIC IMPORTED)
add_executable(link_imported_static_exe ../empty.c)
target_link_libraries(link_imported_static_exe PRIVATE imported_static_lib)

if(NOT CMAKE_GENERATOR STREQUAL "Xcode" OR NOT CMAKE_OSX_ARCHITECTURES MATCHES "[;$]")
  add_library(imported_object_lib OBJECT IMPORTED)
  add_executable(link_imported_object_exe ../empty.c)
  target_link_libraries(link_imported_object_exe PRIVATE imported_object_lib)
endif()

add_library(imported_interface_lib INTERFACE IMPORTED)
add_executable(link_imported_interface_exe ../empty.c)
target_link_libraries(link_imported_interface_exe PRIVATE imported_interface_lib)

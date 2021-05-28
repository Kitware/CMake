project(Imported)

add_library(imported_lib UNKNOWN IMPORTED)
set_target_properties(imported_lib PROPERTIES IMPORTED_LOCATION "imported_unk${CMAKE_STATIC_LIBRARY_SUFFIX}")
add_executable(imported_exe IMPORTED)
add_executable(link_imported_exe ../empty.c)
target_link_libraries(link_imported_exe PRIVATE imported_lib)

add_library(imported_shared_lib SHARED IMPORTED)
set_target_properties(imported_shared_lib PROPERTIES
  IMPORTED_LOCATION "imported_shared${CMAKE_SHARED_LIBRARY_SUFFIX}"
  IMPORTED_IMPLIB "imported_shared${CMAKE_IMPORT_LIBRARY_SUFFIX}"
)
add_executable(link_imported_shared_exe ../empty.c)
target_link_libraries(link_imported_shared_exe PRIVATE imported_shared_lib)

add_library(imported_static_lib STATIC IMPORTED)
set_target_properties(imported_static_lib PROPERTIES IMPORTED_LOCATION "imported_static${CMAKE_STATIC_LIBRARY_SUFFIX}")
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

install(IMPORTED_RUNTIME_ARTIFACTS imported_shared_lib
  DESTINATION lib
)
install(IMPORTED_RUNTIME_ARTIFACTS imported_shared_lib
  DESTINATION lib2 OPTIONAL
)

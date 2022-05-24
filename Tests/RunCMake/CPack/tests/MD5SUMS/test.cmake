install(FILES CMakeLists.txt DESTINATION foo COMPONENT test)
install(FILES CMakeLists.txt DESTINATION bar COMPONENT test)
install(FILES CMakeLists.txt DESTINATION baz COMPONENT test)

if(PACKAGING_TYPE STREQUAL "COMPONENT")
  set(CPACK_COMPONENTS_ALL test)
endif()

if(GENERATOR_TYPE STREQUAL "DEB")
  set(CPACK_PACKAGE_CONTACT "someone")
endif()

install(FILES CMakeLists.txt DESTINATION foo COMPONENT test)

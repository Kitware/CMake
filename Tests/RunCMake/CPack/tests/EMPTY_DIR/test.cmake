if(GENERATOR_TYPE STREQUAL "DEB")
  set(CPACK_PACKAGE_CONTACT "someone")
endif()

if(GENERATOR_TYPE STREQUAL "DEB" OR GENERATOR_TYPE STREQUAL "RPM")
  if(GENERATOR_TYPE STREQUAL "DEB")
    set(generator_type_suffix_ "IAN") # not entirely compatible...
  endif()

  set(CPACK_${GENERATOR_TYPE}${generator_type_suffix_}_FILE_NAME "${GENERATOR_TYPE}-DEFAULT")
endif()

install(DIRECTORY DESTINATION empty
        COMPONENT test)

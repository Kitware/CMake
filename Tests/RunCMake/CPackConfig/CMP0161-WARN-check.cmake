include(${RunCMake_SOURCE_DIR}/check.cmake)

if(DEFINED CPACK_PRODUCTBUILD_DOMANS)
  message(FATAL_ERROR "CPACK_PRODUCTBUILD_DOMANS was defined, but it should not have been")
endif()

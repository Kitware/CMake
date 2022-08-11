file(GLOB contents LIST_DIRECTORIES true "${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/*")

if(NOT contents STREQUAL "")
  list(JOIN contents "\n" fileList)
  message(FATAL_ERROR
    "CMAKE_FIND_PACKAGE_REDIRECTS_DIR is not empty:\n"
    "${fileList}"
  )
endif()

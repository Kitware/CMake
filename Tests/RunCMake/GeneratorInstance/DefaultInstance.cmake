if("x${CMAKE_GENERATOR_INSTANCE}" STREQUAL "x")
  message(FATAL_ERROR "CMAKE_GENERATOR_INSTANCE is empty but should have a value.")
elseif("x${CMAKE_GENERATOR_INSTANCE}" MATCHES [[\\]])
  message(FATAL_ERROR
    "CMAKE_GENERATOR_INSTANCE is\n"
    "  ${CMAKE_GENERATOR_INSTANCE}\n"
    "which contains a backslash.")
elseif(NOT IS_DIRECTORY "${CMAKE_GENERATOR_INSTANCE}")
  message(FATAL_ERROR
    "CMAKE_GENERATOR_INSTANCE is\n"
    "  ${CMAKE_GENERATOR_INSTANCE}\n"
    "which is not an existing directory.")
endif()
file(WRITE "${CMAKE_BINARY_DIR}/instance.txt" "${CMAKE_GENERATOR_INSTANCE}")

message(STATUS "CMAKE_VS_VERSION_BUILD_NUMBER='${CMAKE_VS_VERSION_BUILD_NUMBER}'")

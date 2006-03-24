# Simulate in-source build include-file behavior for out-of-source
# builds.
SET(CMAKE_INCLUDE_CURRENT_DIR 1)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated.h 
  COMMAND 
  ${CMAKE_COMMAND} ARGS -E
  copy ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/generated.h 
)

ADD_LIBRARY(GeneratedHeader main.cpp ${CMAKE_CURRENT_BINARY_DIR}/generated.h) 
 

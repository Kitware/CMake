add_library(libA INTERFACE)
target_include_directories(libA INTERFACE /a/inc)
add_library(libB INTERFACE)
target_include_directories(libB INTERFACE /b/inc1 /b/inc2)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/tp.txt"
  CONTENT "$<LIST:TRANSFORM,libA;libB,APPLY,$<TARGET_PROPERTY:$<_0>,INTERFACE_INCLUDE_DIRECTORIES>>\n")

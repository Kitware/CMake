add_library(bar STATIC bar.cpp)

add_library(foo STATIC foo.cpp)

add_library(baz STATIC foo.cpp)
set_target_properties(baz PROPERTIES EXCLUDE_FROM_ALL OFF)

file(GENERATE
  OUTPUT "${CMAKE_BINARY_DIR}/main.txt"
  CONTENT "$<TARGET_FILE_NAME:main>")

file(GENERATE
  OUTPUT "${CMAKE_BINARY_DIR}/bar.txt"
  CONTENT "$<TARGET_FILE_NAME:bar>")

file(GENERATE
  OUTPUT "${CMAKE_BINARY_DIR}/baz.txt"
  CONTENT "$<TARGET_FILE_NAME:baz>")

target_include_directories(foo PUBLIC .)

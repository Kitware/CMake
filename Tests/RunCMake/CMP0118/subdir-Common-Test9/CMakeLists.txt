file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Generated_source4.txt"
  CONTENT "int func();\nint main(){ return func(); }"
)
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Generated_source5.txt"
  CONTENT "int func();\nint main(){ return func(); }"
)
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Generated_source6.txt"
  CONTENT "int func();\nint main(){ return func(); }"
)


set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source1.txt"
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source2.txt"
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source3.txt"
  TARGET_DIRECTORY custom1
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.txt"
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source5.txt"
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source6.txt"
  TARGET_DIRECTORY custom1
  PROPERTY GENERATED "0")

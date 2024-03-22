# Note: Currently, `file(GENERATE)` does not set the `GENERATED` property!
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.txt"
  CONTENT "int func();\nint main(){ return func(); }"
)
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source5.txt"
  CONTENT "int func();\nint main(){ return func(); }"
)
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source6.txt"
  CONTENT "int func();\nint main(){ return func(); }"
)

add_library(library IMPORTED STATIC)
set_property(TARGET library PROPERTY COMPILE_DEFINITIONS "first")

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/output.txt"
              CONTENT "$<TARGET_PROPERTY:COMPILE_DEFINITIONS>"
              TARGET library
)

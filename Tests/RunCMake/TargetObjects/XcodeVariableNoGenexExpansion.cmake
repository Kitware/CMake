add_library(A OBJECT IMPORTED)

# We don't actually build this example so just configure a dummy
# object file to test.  It does not have to exist.
set_target_properties(A PROPERTIES
  IMPORTED_OBJECTS "${CMAKE_CURRENT_BINARY_DIR}/$(CURRENT_ARCH)/does_not_exist.o"
)

file(GENERATE
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/objects.txt
  CONTENT "$<TARGET_OBJECTS:A>"
)

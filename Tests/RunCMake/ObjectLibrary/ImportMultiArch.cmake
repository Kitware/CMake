
add_library(A OBJECT IMPORTED)

# We don't actually build this example so just configure dummy
# object files to test.  They do not have to exist.
set_target_properties(A PROPERTIES
  IMPORTED_OBJECTS "${CMAKE_CURRENT_BINARY_DIR}/$(CURRENT_ARCH)/does_not_exist.o"
)

add_library(B SHARED $<TARGET_OBJECTS:A> b.c)

# We use this to find the relevant lines of the project.pbx file
target_link_options(B PRIVATE --findconfig-$<CONFIG>)

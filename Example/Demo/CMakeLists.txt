# Make sure the compiler can find include files from our Hello library.
INCLUDE_DIRECTORIES(${HELLO_SOURCE_DIR}/Hello)

# Make sure the linker can find the Hello library once it is built.
LINK_DIRECTORIES(${HELLO_BINARY_DIR}/Hello)

# Add executable called "helloDemo" that is built from the source files
# "demo.cxx" and "demo_b.cxx".  The extensions are automatically found.
ADD_EXECUTABLE(helloDemo demo.cxx demo_b.cxx)

# Link the executable to the Hello library.
TARGET_LINK_LIBRARIES(helloDemo Hello)

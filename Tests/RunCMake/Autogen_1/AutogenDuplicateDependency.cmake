enable_language(CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lib.cpp"
     CONTENT "void foo() {}")
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/main.cpp"
     CONTENT "int main() {return 0;}")

# Test case.
# App depends on Lib, which means App_autogen depends on Lib_autogen by default.
# Which means building App_autogen should trigger the generation of the
# fancy_generated.txt file, which is a dependency of Lib_autogen.

# Create a shared library and an executable.
add_library(Lib SHARED "${CMAKE_CURRENT_BINARY_DIR}/lib.cpp")
add_executable(App "${CMAKE_CURRENT_BINARY_DIR}/main.cpp")

# Link Lib into App more than once. Previously this was not properly handled by AUTOGEN,
# which discarded the Lib_autogen dependency from App_autogen entirely, and the
# file was not generated.
foreach(i RANGE 1 ${LIB_LINK_COUNT})
  target_link_libraries(App PRIVATE Lib)
endforeach()

# Add a custom target that generates a file.
set(generated_file_path "${CMAKE_CURRENT_BINARY_DIR}/fancy_generated.txt")
add_custom_command(
    OUTPUT "${generated_file_path}"
    COMMAND ${CMAKE_COMMAND} -E touch "${generated_file_path}"
)
add_custom_target(generate_file DEPENDS "${generated_file_path}")

# Make sure the file is generated as part of building the Lib_autogen target.
set_target_properties(Lib PROPERTIES
    AUTOGEN_TARGET_DEPENDS generate_file)

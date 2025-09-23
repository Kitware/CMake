enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

# Create a test library with an arbitrary include directory to later override with the property
add_library(foo STATIC ../Autogen_common/example.cpp)
target_include_directories(foo PRIVATE ../Autogen_common/example.h)

# Set AUTOMOC_INCLUDE_DIRECTORIES with a test value to verify it replaces the above include directory
# in AutogenInfo.json's MOC_INCLUDES list.
# See comments in the -check.cmake counterpart for more information about this test.
set_target_properties(foo PROPERTIES
  AUTOMOC ON
  AUTOMOC_INCLUDE_DIRECTORIES "/pass"
)

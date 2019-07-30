cmake_minimum_required(VERSION 3.1...3.14)

project(SwigSrcFileExtension C)

include(CTest)
find_package(SWIG REQUIRED)
find_package(Python COMPONENTS Interpreter Development REQUIRED)

include(${SWIG_USE_FILE})

# Use the newer target name preference
set(UseSWIG_TARGET_NAME_PREFERENCE "STANDARD")

# Set the custom source file extension to both .i and .swg
set(SWIG_SOURCE_FILE_EXTENSIONS ".i" ".swg")

# Generate a Python module out of `.i`
swig_add_library(my_add LANGUAGE python SOURCES my_add.i)
target_link_libraries(my_add Python::Module)

# Generate a Python module out of `.swg`
swig_add_library(my_sub LANGUAGE python SOURCES my_sub.swg)
target_link_libraries(my_sub Python::Module)

# Add a test
add_test(NAME SwigSrcFileExtension
  COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}"
  "${Python_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/runme.py")


enable_language(C)

include(CTest)

cmake_policy(SET CMP0078 NEW)
cmake_policy(SET CMP0086 NEW)

# Development.Module should be enough to get DEBUG_POSTFIX.
find_package(Python REQUIRED COMPONENTS Development.Module)

find_package(SWIG)
include(UseSWIG)

swig_add_library(example LANGUAGE python TYPE MODULE DEBUG_POSTFIX "${Python_DEBUG_POSTFIX}"
                 SOURCES example.i)
target_link_libraries(example PRIVATE Python::Module)


# Path separator
if (WIN32)
  set (PS "$<SEMICOLON>")
else()
  set (PS ":")
endif()

find_package(Python REQUIRED COMPONENTS Interpreter)
add_test (NAME SetPOSTFIX
  COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}${PS}$<TARGET_FILE_DIR:example>"
  "${Python_INTERPRETER}" "${CMAKE_CURRENT_SOURCE_DIR}/runme.py")

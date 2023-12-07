cmake_minimum_required(VERSION 3.5)

project(TestNumPyOnly LANGUAGES C)

if(CMake_TEST_FindPython2_NumPy)

  find_package(Python2 REQUIRED COMPONENTS NumPy)

  Python2_add_library (arraytest2 MODULE ../NumPy/arraytest.c)
  target_compile_definitions (arraytest2 PRIVATE PYTHON2)
  target_link_libraries (arraytest2 PRIVATE Python2::NumPy)

endif()


if(CMake_TEST_FindPython3_NumPy)

find_package(Python3 REQUIRED COMPONENTS NumPy)

Python3_add_library (arraytest3 MODULE ../NumPy/arraytest.c)
target_compile_definitions (arraytest3 PRIVATE PYTHON3)
target_link_libraries (arraytest3 PRIVATE Python3::NumPy)

endif()

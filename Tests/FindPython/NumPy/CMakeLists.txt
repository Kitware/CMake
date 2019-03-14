cmake_minimum_required(VERSION 3.1)

project(TestNumPy C)

find_package (Python2 REQUIRED COMPONENTS Interpreter Development NumPy)
find_package (Python3 REQUIRED COMPONENTS Interpreter Development NumPy)

Python2_add_library (arraytest2 MODULE arraytest.c)
target_compile_definitions (arraytest2 PRIVATE PYTHON2)
target_link_libraries (arraytest2 PRIVATE Python2::NumPy)

Python3_add_library (arraytest3 MODULE arraytest.c)
target_compile_definitions (arraytest3 PRIVATE PYTHON3)
target_link_libraries (arraytest3 PRIVATE Python3::NumPy)

add_test (NAME python2_arraytest
          COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:arraytest2>"
          "${Python2_EXECUTABLE}" -c "import numpy; import arraytest2; arraytest2.vecsq(numpy.array([1, 2, 3]));")

add_test (NAME python3_arraytest
          COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:arraytest3>"
          "${Python3_EXECUTABLE}" -c "import numpy; import arraytest3; arraytest3.vecsq(numpy.array([1, 2, 3]));")

enable_language(C)

include(CTest)

cmake_policy(SET CMP0201 NEW)

find_package (Python3 REQUIRED COMPONENTS Interpreter Development.SABIModule NumPy)

Python3_add_library (arraytest3 MODULE USE_SABI 3.${Python3_VERSION_MINOR} WITH_SOABI arraytest.c)
target_compile_definitions (arraytest3 PRIVATE PYTHON3)
target_link_libraries (arraytest3 PRIVATE Python3::NumPy)

add_test (NAME python3_arraytest
  COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:arraytest3>"
  "${Python3_INTERPRETER}" -c "import numpy; import arraytest3; arraytest3.vecsq(numpy.array([1, 2, 3]));")

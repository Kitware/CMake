cmake_minimum_required(VERSION 3.1)

project(TestPython3 C)

include(CTest)

find_package(Python3 2 QUIET)
if (Python3_FOUND)
  message (FATAL_ERROR "Wrong python version found: ${Python3_VERSION}")
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Fail to found Python 3")
endif()

if(NOT TARGET Python3::Interpreter)
  message(SEND_ERROR "Python2::Interpreter not found")
endif()

if(NOT TARGET Python3::Python)
  message(SEND_ERROR "Python2::Python not found")
endif()
if(NOT TARGET Python3::Module)
  message(SEND_ERROR "Python2::Module not found")
endif()

Python3_add_library (spam3 MODULE ../spam.c)
target_compile_definitions (spam3 PRIVATE PYTHON3)

add_test (NAME python3_spam3
          COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:spam3>"
          "${Python3_EXECUTABLE}" -c "import spam3; spam3.system(\"cd\")")

add_test(NAME findpython3_script
         COMMAND "${CMAKE_COMMAND}" -DPYTHON_PACKAGE_NAME=Python3
         -P "${CMAKE_CURRENT_LIST_DIR}/../FindPythonScript.cmake")

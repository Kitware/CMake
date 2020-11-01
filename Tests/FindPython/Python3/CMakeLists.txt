cmake_minimum_required(VERSION 3.1)

project(TestPython3 LANGUAGES C)

include(CTest)

find_package(Python3 2 QUIET)
if (Python3_FOUND)
  message (FATAL_ERROR "Wrong python version found: ${Python3_VERSION}")
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Failed to find Python 3")
endif()
if (NOT Python3_Development_FOUND)
  message (FATAL_ERROR "Failed to find Python 3 'Development' component")
endif()
if (NOT Python3_Development.Module_FOUND)
  message (FATAL_ERROR "Failed to find Python 3 'Development.Module' component")
endif()
if (NOT Python3_Development.Embed_FOUND)
  message (FATAL_ERROR "Failed to find Python 3 'Development.Embed' component")
endif()

if(NOT TARGET Python3::Interpreter)
  message(SEND_ERROR "Python3::Interpreter not found")
endif()

if(NOT TARGET Python3::Python)
  message(SEND_ERROR "Python3::Python not found")
endif()
if(NOT TARGET Python3::Module)
  message(SEND_ERROR "Python3::Module not found")
endif()

Python3_add_library (spam3 MODULE ../spam.c)
target_compile_definitions (spam3 PRIVATE PYTHON3)

add_test (NAME python3_spam3
          COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:spam3>"
          "${Python3_EXECUTABLE}" -c "import spam3; spam3.system(\"cd\")")

add_test(NAME findpython3_script
         COMMAND "${CMAKE_COMMAND}" -DPYTHON_PACKAGE_NAME=Python3
         -DPython3_FIND_STRATEGY=${Python3_FIND_STRATEGY}
         -P "${CMAKE_CURRENT_LIST_DIR}/../FindPythonScript.cmake")


## Try a new search specifying only expected ABI
# retrieve ABI of python interpreter
execute_process (COMMAND "${Python3_EXECUTABLE}" -c
                         "import sys; sys.stdout.write(sys.abiflags)"
                 RESULT_VARIABLE result
                 OUTPUT_VARIABLE abi
                 ERROR_QUIET
                 OUTPUT_STRIP_TRAILING_WHITESPACE)
if (result)
  # assume ABI is not supported
  set (abi "")
endif()

# define FIND_ABI variable
if (abi MATCHES "d")
  set (Python3_VALID_ABI "ON")
else()
  set (Python3_VALID_ABI "OFF")
endif()
if (abi MATCHES "m")
  list (APPEND Python3_VALID_ABI "ON")
else()
  list (APPEND Python3_VALID_ABI "OFF")
endif()
if (abi MATCHES "u")
  list (APPEND Python3_VALID_ABI "ON")
else()
  list (APPEND Python3_VALID_ABI "OFF")
endif()
# build an invalid pattern for ABI
set (Python3_INVALID_ABI)
foreach (abi IN LISTS Python3_VALID_ABI)
  if (abi)
    list (APPEND Python3_INVALID_ABI "OFF")
  else()
    list (APPEND Python3_INVALID_ABI "ON")
  endif()
endforeach()

add_test(NAME python3_find_valid_abi
         COMMAND "${CMAKE_COMMAND}" -DPYTHON_PACKAGE_NAME=Python3
         -DPython3_FIND_STRATEGY=${Python3_FIND_STRATEGY}
         "-DPython3_FIND_ABI=${Python3_VALID_ABI}"
         -P "${CMAKE_CURRENT_LIST_DIR}/../FindPythonScript.cmake")
add_test(NAME python3_find_invalid_abi
         COMMAND "${CMAKE_COMMAND}" -DPYTHON_PACKAGE_NAME=Python3
         -DPYTHON_MUST_NOT_BE_FOUND=ON
         -DPython3_FIND_STRATEGY=${Python3_FIND_STRATEGY}
         "-DPython3_FIND_ABI=${Python3_INVALID_ABI}"
         -P "${CMAKE_CURRENT_LIST_DIR}/../FindPythonScript.cmake")


enable_language(C)

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

Python3_add_library (spam3 MODULE spam.c)
target_compile_definitions (spam3 PRIVATE PYTHON3)

add_test (NAME python3_spam3
          COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:spam3>"
          "${Python3_INTERPRETER}" -c "import spam3; spam3.system(\"cd\")")

add_test(NAME findpython3_script
         COMMAND "${CMAKE_COMMAND}" -DPYTHON_PACKAGE_NAME=Python3
         -DPython3_FIND_STRATEGY=${Python3_FIND_STRATEGY}
         -P "${CMAKE_CURRENT_LIST_DIR}/FindPythonScript.cmake")

#
# New search with user's prefix
#
foreach(item IN ITEMS FOUND Development_FOUND Development.Module_FOUND Development.Embed_FOUND)
  unset(Python3_${item})
endforeach()

set(Python3_ARTIFACTS_PREFIX "_TEST")
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
if (NOT Python3_TEST_FOUND OR NOT Python3_FOUND)
  message (FATAL_ERROR "Failed to find Python 3 (TEST prefix)")
endif()
if (NOT Python3_TEST_Development_FOUND OR NOT Python3_Development_FOUND)
  message (FATAL_ERROR "Failed to find Python 3 'Development' component (TEST prefix)")
endif()
if (NOT Python3_TEST_Development.Module_FOUND OR NOT Python3_Development.Module_FOUND)
  message (FATAL_ERROR "Failed to find Python 3 'Development.Module' component (TEST prefix)")
endif()
if (NOT Python3_TEST_Development.Embed_FOUND OR NOT Python3_Development.Embed_FOUND)
  message (FATAL_ERROR "Failed to find Python 3 'Development.Embed' component (TEST prefix)")
endif()

if(NOT TARGET Python3_TEST::Interpreter)
  message(SEND_ERROR "Python3_TEST::Interpreter not found")
endif()

if(NOT TARGET Python3_TEST::Python)
  message(SEND_ERROR "Python3_TEST::Python not found")
endif()
if(NOT TARGET Python3_TEST::Module)
  message(SEND_ERROR "Python3_TEST::Module not found")
endif()

Python3_TEST_add_library (TEST_spam3 MODULE TEST_spam.c)
target_compile_definitions (TEST_spam3 PRIVATE PYTHON3)

add_test (NAME python3_TEST_spam3
          COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:TEST_spam3>"
          "${Python3_INTERPRETER}" -c "import TEST_spam3; TEST_spam3.system(\"cd\")")

cmake_minimum_required(VERSION 3.15)

project(TestPython LANGUAGES C)

include(CTest)

find_package(Python ${Python_REQUESTED_VERSION} REQUIRED COMPONENTS Interpreter Development)
if (NOT Python_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION}")
endif()
if (NOT Python_Development.Module_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION}, COMPONENT 'Development.Module'")
endif()
if (NOT Python_Development.Embed_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION}, COMPOENENT 'Development.Embed'")
endif()

if(NOT TARGET Python::Interpreter)
  message(SEND_ERROR "Python::Interpreter not found")
endif()

if(NOT TARGET Python::Python)
  message(SEND_ERROR "Python::Python not found")
endif()
if(NOT TARGET Python::Module)
  message(SEND_ERROR "Python::Module not found")
endif()

if (Python_REQUESTED_VERSION)
  Python_add_library (spam${Python_REQUESTED_VERSION} MODULE ../spam.c)
  target_compile_definitions (spam${Python_REQUESTED_VERSION} PRIVATE PYTHON${Python_REQUESTED_VERSION})

  add_test (NAME python_spam${Python_REQUESTED_VERSION}
            COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:spam${Python_REQUESTED_VERSION}>"
            "${Python_INTERPRETER}" -c "import spam${Python_REQUESTED_VERSION}; spam${Python_REQUESTED_VERSION}.system(\"cd\")")
else()
  add_test(NAME findpython_script
           COMMAND "${CMAKE_COMMAND}" -DPYTHON_PACKAGE_NAME=Python
           -P "${CMAKE_CURRENT_LIST_DIR}/../FindPythonScript.cmake")
endif()


#
# New search with user's prefix
#
foreach(item IN ITEMS FOUND Development_FOUND Development.Module_FOUND Development.Embed_FOUND)
  unset(Python_${item})
endforeach()

set(Python_ARTIFACTS_PREFIX "_TEST")
find_package(Python ${Python_REQUESTED_VERSION} REQUIRED COMPONENTS Interpreter Development)
if (NOT Python_TEST_FOUND OR NOT Python_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION} (TEST prefix)")
endif()
if (NOT Python_TEST_Development.Module_FOUND OR NOT Python_Development.Module_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION}, COMPONENT 'Development.Module' (TEST prefix)")
endif()
if (NOT Python_TEST_Development.Embed_FOUND OR NOT Python_Development.Embed_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION}, COMPOENENT 'Development.Embed' (TEST prefix)")
endif()

if(NOT TARGET Python_TEST::Interpreter)
  message(SEND_ERROR "Python_TEST::Interpreter not found")
endif()

if(NOT TARGET Python_TEST::Python)
  message(SEND_ERROR "Python_TEST::Python not found")
endif()
if(NOT TARGET Python_TEST::Module)
  message(SEND_ERROR "Python_TEST::Module not found")
endif()

if (Python_REQUESTED_VERSION)
  Python_TEST_add_library (TEST_spam${Python_REQUESTED_VERSION} MODULE ../TEST_spam.c)
  target_compile_definitions (TEST_spam${Python_REQUESTED_VERSION} PRIVATE PYTHON${Python_REQUESTED_VERSION})

  add_test (NAME python_TEST_spam${Python_REQUESTED_VERSION}
            COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:TEST_spam${Python_REQUESTED_VERSION}>"
            "${Python_INTERPRETER}" -c "import TEST_spam${Python_REQUESTED_VERSION}; TEST_spam${Python_REQUESTED_VERSION}.system(\"cd\")")
endif()

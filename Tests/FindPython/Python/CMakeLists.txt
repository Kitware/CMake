cmake_minimum_required(VERSION 3.5)

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
            "${Python_EXECUTABLE}" -c "import spam${Python_REQUESTED_VERSION}; spam${Python_REQUESTED_VERSION}.system(\"cd\")")
else()
  add_test(NAME findpython_script
           COMMAND "${CMAKE_COMMAND}" -DPYTHON_PACKAGE_NAME=Python
           -P "${CMAKE_CURRENT_LIST_DIR}/../FindPythonScript.cmake")
endif()

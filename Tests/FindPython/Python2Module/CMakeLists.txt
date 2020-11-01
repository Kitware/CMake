cmake_minimum_required(VERSION 3.1)

project(TestPython2Module LANGUAGES C)

include(CTest)

find_package(Python2 REQUIRED COMPONENTS Interpreter Development.Module)
if (NOT Python2_FOUND)
  message (FATAL_ERROR "Failed to find Python 2")
endif()
if (Python2_Development_FOUND)
  message (FATAL_ERROR "Python 2, COMPONENT 'Development' unexpectedly found")
endif()
if (Python2_Development.Embed_FOUND)
  message (FATAL_ERROR "Python 2, COMPONENT 'Development.Embed' unexpectedly found")
endif()
if (NOT Python2_Development.Module_FOUND)
  message (FATAL_ERROR "Python 2, COMPONENT 'Development.Module' not found")
endif()

if(NOT TARGET Python2::Interpreter)
  message(SEND_ERROR "Python2::Interpreter not found")
endif()

if(TARGET Python2::Python)
  message(SEND_ERROR "Python2::Python unexpectedly found")
endif()
if(NOT TARGET Python2::Module)
  message(SEND_ERROR "Python2::Module not found")
endif()

Python2_add_library (spam2 MODULE ../spam.c)
target_compile_definitions (spam2 PRIVATE PYTHON2)

add_test (NAME python2_spam2
          COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:spam2>"
          "${Python2_EXECUTABLE}" -c "import spam2; spam2.system(\"cd\")")

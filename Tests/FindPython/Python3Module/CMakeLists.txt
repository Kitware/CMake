cmake_minimum_required(VERSION 3.5)

project(TestPython3Module LANGUAGES C)

include(CTest)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Failed to find Python 3")
endif()
if (Python3_Development_FOUND)
  message (FATAL_ERROR "Python 3, COMPONENT 'Development' unexpectedly found")
endif()
if (Python3_Development.SABIModule_FOUND)
  message (FATAL_ERROR "Python 3, COMPONENT 'Development.SABIModule' unexpectedly found")
endif()
if (Python3_Development.Embed_FOUND)
  message (FATAL_ERROR "Python 3, COMPONENT 'Development.Embed' unexpectedly found")
endif()
if (NOT Python3_Development.Module_FOUND)
  message (FATAL_ERROR "Python 3, COMPONENT 'Development.Module' not found")
endif()

if(NOT TARGET Python3::Interpreter)
  message(SEND_ERROR "Python3::Interpreter not found")
endif()

if(TARGET Python3::Python)
  message(SEND_ERROR "Python3::Python unexpectedly found")
endif()
if(TARGET Python3::SABIMOdule)
  message(SEND_ERROR "Python3::SABIModule unexpectedly found")
endif()
if(TARGET Python3::Embed)
  message(SEND_ERROR "Python3::Embed unexpectedly found")
endif()
if(NOT TARGET Python3::Module)
  message(SEND_ERROR "Python3::Module not found")
endif()

Python3_add_library (spam3 MODULE ../spam.c)
target_compile_definitions (spam3 PRIVATE PYTHON3)

add_test (NAME python3_spam3
          COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:spam3>"
          "${Python3_EXECUTABLE}" -c "import spam3; spam3.system(\"cd\")")

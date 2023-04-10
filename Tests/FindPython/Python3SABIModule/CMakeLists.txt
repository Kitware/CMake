cmake_minimum_required(VERSION 3.7)

project(TestPython3SABIModule LANGUAGES C)

include(CTest)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development.SABIModule)
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Failed to find Python 3")
endif()
if (Python3_Development_FOUND)
  message (FATAL_ERROR "Python 3, COMPONENT 'Development' unexpectedly found")
endif()
if (Python3_Development.Embed_FOUND)
  message (FATAL_ERROR "Python 3, COMPONENT 'Development.Embed' unexpectedly found")
endif()
if (Python3_Development.Module_FOUND)
  message (FATAL_ERROR "Python 3, COMPONENT 'Development.Module' unexpectedly found")
endif()
if (NOT Python3_Development.SABIModule_FOUND)
  message (FATAL_ERROR "Python 3, COMPONENT 'Development.SABIModule' not found")
endif()

if(NOT TARGET Python3::Interpreter)
  message(SEND_ERROR "Python3::Interpreter not found")
endif()

if(TARGET Python3::Python)
  message(SEND_ERROR "Python3::Python unexpectedly found")
endif()
if(TARGET Python3::Module)
  message(SEND_ERROR "Python3::Module unexpectedly found")
endif()
if(NOT TARGET Python3::SABIModule)
  message(SEND_ERROR "Python3::SABIModule not found")
endif()

if (Python3_VERSION VERSION_GREATER_EQUAL "3.2" AND NOT Python3_SOSABI)
  message(FATAL_ERROR "Python3_SOSABI unexpectedly not defined")
endif()

Python3_add_library (spam3 MODULE USE_SABI 3 WITH_SOABI ../spam.c)
target_compile_definitions (spam3 PRIVATE PYTHON3)

if (Python3_SOSABI)
  get_property (suffix TARGET spam3 PROPERTY SUFFIX)
  if (NOT suffix MATCHES "^\\.${Python3_SOSABI}")
    message(FATAL_ERROR "Module suffix do not include Python3_SOSABI")
  endif()
endif()


add_test (NAME python3_spam3
          COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:spam3>"
          "${Python3_EXECUTABLE}" -c "import spam3; spam3.system(\"cd\")")

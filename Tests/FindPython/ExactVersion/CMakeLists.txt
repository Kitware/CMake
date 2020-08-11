cmake_minimum_required(VERSION 3.1)

project(TestExactVersion C)

find_package(Python${Python_MAJOR_VERSION} ${Python_REQUESTED_VERSION} COMPONENTS Interpreter Development)
if (NOT Python${Python_MAJOR_VERSION}_FOUND)
  message (FATAL_ERROR "Fail to found Python ${Python_REQUESTED_VERSION}")
endif()
if (NOT Python${Python_MAJOR_VERSION}_Interpreter_FOUND)
  message (FATAL_ERROR "Fail to found Python ${Python_REQUESTED_VERSION} Interpreter")
endif()
if (NOT Python${Python_MAJOR_VERSION}_Development_FOUND)
  message (FATAL_ERROR "Fail to found Python ${Python_REQUESTED_VERSION} Development")
endif()

if(NOT TARGET Python${Python_MAJOR_VERSION}::Interpreter)
  message(SEND_ERROR "Python${Python_MAJOR_VERSION}::Interpreter not found")
endif()

if(NOT TARGET Python${Python_MAJOR_VERSION}::Python)
  message(SEND_ERROR "Python${Python_MAJOR_VERSION}::Python not found")
endif()
if(NOT TARGET Python${Python_MAJOR_VERSION}::Module)
  message(SEND_ERROR "Python${Python_MAJOR_VERSION}::Module not found")
endif()


# reset artifacts and second search with exact version already founded
unset (Python${Python_MAJOR_VERSION}_EXECUTABLE)
unset (_Python${Python_MAJOR_VERSION}_EXECUTABLE CACHE)

unset (_Python${Python_MAJOR_VERSION}_LIBRARY_RELEASE CACHE)
unset (_Python${Python_MAJOR_VERSION}_INCLUDE_DIR CACHE)

set (Python_REQUESTED_VERSION ${Python${Python_MAJOR_VERSION}_VERSION})
find_package(Python${Python_MAJOR_VERSION} ${Python_REQUESTED_VERSION} EXACT COMPONENTS Interpreter Development)
if (NOT Python${Python_MAJOR_VERSION}_FOUND)
  message (FATAL_ERROR "Fail to found Python ${Python_REQUESTED_VERSION}")
endif()
if (NOT Python${Python_MAJOR_VERSION}_Interpreter_FOUND)
  message (FATAL_ERROR "Fail to found Python ${Python_REQUESTED_VERSION} Interpreter")
endif()
if (NOT Python${Python_MAJOR_VERSION}_Development_FOUND)
  message (FATAL_ERROR "Fail to found Python ${Python_REQUESTED_VERSION} Development")
endif()

if(NOT TARGET Python${Python_MAJOR_VERSION}::Interpreter)
  message(SEND_ERROR "Python${Python_MAJOR_VERSION}::Interpreter not found")
endif()

if(NOT TARGET Python${Python_MAJOR_VERSION}::Python)
  message(SEND_ERROR "Python${Python_MAJOR_VERSION}::Python not found")
endif()
if(NOT TARGET Python${Python_MAJOR_VERSION}::Module)
  message(SEND_ERROR "Python${Python_MAJOR_VERSION}::Module not found")
endif()

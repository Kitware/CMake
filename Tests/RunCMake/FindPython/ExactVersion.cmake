enable_language(C)

find_package(${PYTHON} ${Python_REQUESTED_VERSION} COMPONENTS Interpreter Development)
if(NOT ${PYTHON}_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION}")
endif()
if(NOT ${PYTHON}_Interpreter_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION} Interpreter")
endif()
if(NOT ${PYTHON}_Development_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION} Development")
endif()

if(NOT TARGET ${PYTHON}::Interpreter)
  message(SEND_ERROR "${PYTHON}::Interpreter not found")
endif()

if(NOT TARGET ${PYTHON}::Python)
  message(SEND_ERROR "${PYTHON}::Python not found")
endif()
if(NOT TARGET ${PYTHON}::Module)
  message(SEND_ERROR "${PYTHON}::Module not found")
endif()


# reset artifacts and second search with exact version already founded
unset(${PYTHON}_EXECUTABLE)
unset(_${PYTHON}_EXECUTABLE CACHE)

unset(_${PYTHON}_LIBRARY_RELEASE CACHE)
unset(_${PYTHON}_INCLUDE_DIR CACHE)

set(Python_REQUESTED_VERSION ${${PYTHON}_VERSION})
find_package(${PYTHON} ${Python_REQUESTED_VERSION} EXACT COMPONENTS Interpreter Development)
if(NOT ${PYTHON}_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION}")
endif()
if(NOT ${PYTHON}_Interpreter_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION} Interpreter")
endif()
if(NOT ${PYTHON}_Development_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION} Development")
endif()

if(NOT TARGET ${PYTHON}::Interpreter)
  message(SEND_ERROR "${PYTHON}::Interpreter not found")
endif()

if(NOT TARGET ${PYTHON}::Python)
  message(SEND_ERROR "${PYTHON}::Python not found")
endif()
if(NOT TARGET ${PYTHON}::Module)
  message(SEND_ERROR "${PYTHON}::Module not found")
endif()

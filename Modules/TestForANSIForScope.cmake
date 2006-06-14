# - Check for ANSI for scope support
# Check if the compiler supports std:: on stl classes.
#  CMAKE_NO_STD_NAMESPACE - holds result
#

IF("CMAKE_ANSI_FOR_SCOPE" MATCHES "^CMAKE_ANSI_FOR_SCOPE$")
  MESSAGE(STATUS "Check for ANSI scope")
  TRY_COMPILE(CMAKE_ANSI_FOR_SCOPE  ${CMAKE_BINARY_DIR} 
    ${CMAKE_ROOT}/Modules/TestForAnsiForScope.cxx
    OUTPUT_VARIABLE OUTPUT)
  IF (CMAKE_ANSI_FOR_SCOPE)
    MESSAGE(STATUS "Check for ANSI scope - found")
    SET (CMAKE_NO_ANSI_FOR_SCOPE 0 CACHE INTERNAL 
      "Does the compiler support ansi for scope.")
    FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if the CXX compiler understands ansi for scopes passed with "
      "the following output:\n${OUTPUT}\n\n")
  ELSE (CMAKE_ANSI_FOR_SCOPE)
    MESSAGE(STATUS "Check for ANSI scope - not found")
    SET (CMAKE_NO_ANSI_FOR_SCOPE 1 CACHE INTERNAL 
      "Does the compiler support ansi for scope.")
    FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "Determining if the CXX compiler understands ansi for scopes failed with "
      "the following output:\n${OUTPUT}\n\n")
  ENDIF (CMAKE_ANSI_FOR_SCOPE)
ENDIF("CMAKE_ANSI_FOR_SCOPE" MATCHES "^CMAKE_ANSI_FOR_SCOPE$")






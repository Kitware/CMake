#
# check if the compiler supports std:: on stl classes
#
# CMAKE_NO_STD_NAMESPACE - defined accoreding to the results
#

TRY_COMPILE(CMAKE_ANSI_FOR_SCOPE  ${PROJECT_BINARY_DIR} 
  ${CMAKE_ROOT}/Modules/TestForAnsiForScope.cxx)
IF (CMAKE_ANSI_FOR_SCOPE)
  SET (CMAKE_NO_ANSI_FOR_SCOPE 0 CACHE INTERNAL 
       "Does the compiler support ansi for scope.")
ELSE (CMAKE_ANSI_FOR_SCOPE)
  SET (CMAKE_NO_ANSI_FOR_SCOPE 1 CACHE INTERNAL 
     "Does the compiler support ansi for scope.")
ENDIF (CMAKE_ANSI_FOR_SCOPE)





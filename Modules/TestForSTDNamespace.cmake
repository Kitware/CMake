#
# check if the compiler supports std:: on stl classes
#
# CMAKE_NO_STD_NAMESPACE - defined accoreding to the results
#

TRY_COMPILE(CMAKE_STD_NAMESPACE  ${PROJECT_BINARY_DIR} 
  ${CMAKE_ROOT}/Modules/TestForSTDNamespace.cxx)
IF (CMAKE_STD_NAMESPACE)
  SET (CMAKE_NO_STD_NAMESPACE 0 CACHE INTERNAL 
       "Does the compiler support std::.")
ELSE (CMAKE_STD_NAMESPACE)
  SET (CMAKE_NO_STD_NAMESPACE 1 CACHE INTERNAL 
     "Does the compiler support std::.")
ENDIF (CMAKE_STD_NAMESPACE)




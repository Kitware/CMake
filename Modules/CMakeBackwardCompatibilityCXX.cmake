#
INCLUDE(${CMAKE_ROOT}/Modules/TestForANSIStreamHeaders.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/TestForANSIForScope.cmake)
CHECK_INCLUDE_FILE_CXX("sstream" CMAKE_HAS_ANSI_STRING_STREAM)
IF(NOT CMAKE_HAS_ANSI_STRING_STREAM)
  SET(  CMAKE_NO_ANSI_STRING_STREAM 1 CACHE INTERNAL 
       "Does the compiler support sstream or stringstream.")
ENDIF(NOT CMAKE_HAS_ANSI_STRING_STREAM)

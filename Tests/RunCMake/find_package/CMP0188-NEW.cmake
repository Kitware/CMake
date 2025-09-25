cmake_policy(SET CMP0188 NEW)
set(_FindGCCXML_testing TRUE)
find_package(GCCXML MODULE)

if(_FindGCCXML_included)
  message(FATAL_ERROR "FindGCCXML.cmake erroneously included")
endif()

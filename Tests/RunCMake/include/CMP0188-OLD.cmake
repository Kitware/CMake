cmake_policy(SET CMP0188 OLD)
set(_FindGCCXML_testing 1)
include(FindGCCXML)

if(NOT _FindGCCXML_included)
  message(FATAL_ERROR "FindGCCXML.cmake not included")
endif()

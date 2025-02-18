# Do not set CMP0188.
set(_FindGCCXML_testing 1)
include(FindGCCXML)

if(NOT _FindGCCXML_included)
  message(FATAL_ERROR "FindGCCXML.cmake not included")
endif()

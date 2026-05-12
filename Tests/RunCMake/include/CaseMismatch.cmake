list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CaseMismatch")
include(casemodule)

if(NOT case_module_included)
  message(FATAL_ERROR "CaseModule.cmake not included")
endif()

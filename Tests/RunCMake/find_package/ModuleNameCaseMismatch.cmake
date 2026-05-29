list(PREPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_LIST_DIR}/ModuleNameCaseMismatch")
find_package(casemodule MODULE)

if(NOT casemodule_FOUND)
  message(FATAL_ERROR "FindCaseModule.cmake not included")
endif()

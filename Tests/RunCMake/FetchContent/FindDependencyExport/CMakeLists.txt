if(NOT CMAKE_EXPORT_FIND_PACKAGE_NAME STREQUAL _expected_export_find_package_name_sub)
  message(FATAL_ERROR "Expected value of CMAKE_EXPORT_FIND_PACKAGE_NAME:\n  ${_expected_export_find_package_name_sub}\nActual value:\n  ${CMAKE_EXPORT_FIND_PACKAGE_NAME}")
endif()

set(sub_called TRUE PARENT_SCOPE)

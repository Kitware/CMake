function(fde_provide_dependency method name)
  if(NOT CMAKE_EXPORT_FIND_PACKAGE_NAME STREQUAL _expected_export_find_package_name_dp)
    message(FATAL_ERROR "Expected value of CMAKE_EXPORT_FIND_PACKAGE_NAME:\n  ${_expected_export_find_package_name_dp}\nActual value:\n  ${CMAKE_EXPORT_FIND_PACKAGE_NAME}")
  endif()

  set(dp_called TRUE PARENT_SCOPE)
endfunction()

cmake_language(SET_DEPENDENCY_PROVIDER fde_provide_dependency
  SUPPORTED_METHODS FETCHCONTENT_MAKEAVAILABLE_SERIAL
  )

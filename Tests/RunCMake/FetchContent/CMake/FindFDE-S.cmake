if(NOT CMAKE_EXPORT_FIND_PACKAGE_NAME STREQUAL "SomeOtherValue")
  message(FATAL_ERROR "Expected value of CMAKE_EXPORT_FIND_PACKAGE_NAME:\n  SomeOtherValue\nActual value:\n  ${CMAKE_EXPORT_FIND_PACKAGE_NAME}")
endif()

set(fp_called TRUE)
set(FDE-S_FOUND TRUE)

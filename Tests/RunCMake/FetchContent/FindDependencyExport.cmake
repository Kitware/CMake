include(FetchContent)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

unset(dp_called)
unset(fp_called)
set(_expected_export_find_package_name_dp FDE-U)
FetchContent_Declare(
  FDE-U
  FIND_PACKAGE_ARGS
  )
FetchContent_MakeAvailable(FDE-U)

if(NOT dp_called)
  message(FATAL_ERROR "FetchContent_MakeAvailable did not call dependency provider")
endif()
if(NOT fp_called)
  message(FATAL_ERROR "FetchContent_MakeAvailable did not call find_package()")
endif()

if(DEFINED CMAKE_EXPORT_FIND_PACKAGE_NAME)
  message(FATAL_ERROR "CMAKE_EXPORT_FIND_PACKAGE_NAME should have been unset after FetchContent_MakeAvailable().\nActual value:\n  ${CMAKE_EXPORT_FIND_PACKAGE_NAME}")
endif()

unset(sub_called)
set(_expected_export_find_package_name_dp FDE-U-Sub)
set(_expected_export_find_package_name_sub FDE-U-Sub)
FetchContent_Declare(
  FDE-U-Sub
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FindDependencyExport
  )
FetchContent_MakeAvailable(FDE-U-Sub)

if(NOT sub_called)
  message(FATAL_ERROR "FetchContent_MakeAvailable did not call add_subdirectory()")
endif()

if(DEFINED CMAKE_EXPORT_FIND_PACKAGE_NAME)
  message(FATAL_ERROR "CMAKE_EXPORT_FIND_PACKAGE_NAME should have been unset after FetchContent_MakeAvailable()")
endif()

unset(dp_called)
unset(fp_called)
set(CMAKE_EXPORT_FIND_PACKAGE_NAME SomeOtherValue)
set(_expected_export_find_package_name_dp FDE-S)
FetchContent_Declare(
  FDE-S
  FIND_PACKAGE_ARGS
  )
FetchContent_MakeAvailable(FDE-S)

if(NOT dp_called)
  message(FATAL_ERROR "FetchContent_MakeAvailable did not call dependency provider")
endif()
if(NOT fp_called)
  message(FATAL_ERROR "FetchContent_MakeAvailable did not call find_package()")
endif()

if(NOT CMAKE_EXPORT_FIND_PACKAGE_NAME STREQUAL "SomeOtherValue")
  message(FATAL_ERROR "Expected value of CMAKE_EXPORT_FIND_PACKAGE_NAME:\n  SomeOtherValue\nActual value:\n  ${CMAKE_EXPORT_FIND_PACKAGE_NAME}")
endif()

unset(sub_called)
set(_expected_export_find_package_name_dp FDE-S-Sub)
set(_expected_export_find_package_name_sub FDE-S-Sub)
FetchContent_Declare(
  FDE-S-Sub
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FindDependencyExport
  )
FetchContent_MakeAvailable(FDE-S-Sub)

if(NOT sub_called)
  message(FATAL_ERROR "FetchContent_MakeAvailable did not call add_subdirectory()")
endif()

if(NOT CMAKE_EXPORT_FIND_PACKAGE_NAME STREQUAL "SomeOtherValue")
  message(FATAL_ERROR "Expected value of CMAKE_EXPORT_FIND_PACKAGE_NAME:\n  SomeOtherValue\nActual value:\n  ${CMAKE_EXPORT_FIND_PACKAGE_NAME}")
endif()

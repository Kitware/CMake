include(${CMAKE_CURRENT_LIST_DIR}/check.cmake)
find_package(FLEX REQUIRED)

if(NOT FLEX_VERSION MATCHES "^[0-9]+\\.[0-9]+")
  message(SEND_ERROR "FLEX_VERSION is not a version:\n  \"${FLEX_VERSION}\"")
endif()

flex_target(C scanner.l scanner.c)
check_var(FLEX_C_DEFINED TRUE)
check_var(FLEX_C_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scanner.l")
check_var(FLEX_C_OUTPUT_HEADER "")
check_var(FLEX_C_OUTPUTS "${CMAKE_CURRENT_BINARY_DIR}/scanner.c")
check_var(FLEX_C_OPTIONS "")
check_var(FLEX_C_COMPILE_FLAGS "")

flex_target(Subdir scanner.l sub/scanner.c)
check_var(FLEX_Subdir_OUTPUTS "${CMAKE_CURRENT_BINARY_DIR}/sub/scanner.c")

set(out "${CMAKE_CURRENT_BINARY_DIR}/absolute.c")
flex_target(Absolute "${CMAKE_CURRENT_SOURCE_DIR}/scanner.l" "${out}")
check_var(FLEX_Absolute_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scanner.l")
check_var(FLEX_Absolute_OUTPUTS "${out}")

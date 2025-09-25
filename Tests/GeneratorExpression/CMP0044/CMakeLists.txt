
string(TOLOWER ${CMAKE_C_COMPILER_ID} lc_test)
if (lc_test STREQUAL CMAKE_C_COMPILER_ID)
  string(TOUPPER ${CMAKE_C_COMPILER_ID} lc_test)
  if (lc_test STREQUAL CMAKE_C_COMPILER_ID)
    message(SEND_ERROR "Try harder.")
  endif()
endif()

add_library(cmp0044-check cmp0044-check.cpp)
target_compile_definitions(cmp0044-check
  PRIVATE
    Result=$<C_COMPILER_ID:${lc_test}>
)

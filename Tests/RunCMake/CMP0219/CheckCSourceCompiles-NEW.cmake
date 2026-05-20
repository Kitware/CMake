cmake_policy(SET CMP0219 NEW)

enable_language(C)
include(CheckCSourceCompiles)
check_c_source_compiles("int main() { return '\\0'; }" RESULT)
if(NOT RESULT)
  message(FATAL_ERROR "check_c_source_compiles failed for escaped NUL character source")
endif()

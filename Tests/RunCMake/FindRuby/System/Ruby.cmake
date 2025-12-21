enable_language(C)

cmake_policy(SET CMP0185 NEW)

set(Ruby_RBENV_EXECUTABLE "") # Suppress rbenv code path for this test.

find_package(Ruby 1.9.9 REQUIRED)
if (NOT Ruby_FOUND)
  message (FATAL_ERROR "Failed to find Ruby >=1.9.9")
endif()

if (NOT Ruby_Interpreter_FOUND)
  message (FATAL_ERROR "Failed to find Ruby 'Interpreter' component")
endif()

if (NOT Ruby_Development_FOUND)
  message (FATAL_ERROR "Failed to find Ruby 'Development' component")
endif()

foreach(var_CMP0185
    RUBY_EXECUTABLE
    RUBY_INCLUDE_DIRS
    RUBY_LIBRARY
    RUBY_VERSION
    )
  if(DEFINED ${var_CMP0185})
    message(FATAL_ERROR "Pre-CMP0185 result variable is set: ${var_CMP0185}")
  endif()
endforeach()

if(NOT TARGET Ruby::Interpreter)
  message(SEND_ERROR "Ruby::Interpreter not found")
endif()

if (NOT TARGET Ruby::Ruby)
  message(SEND_ERROR "Ruby::Ruby not found")
endif()

if (NOT TARGET Ruby::Module)
  message(SEND_ERROR "Ruby::Module not found")
endif()

add_executable(ruby_version_var "${CMAKE_CURRENT_LIST_DIR}/ruby_version.c")
target_include_directories(ruby_version_var PRIVATE ${Ruby_INCLUDE_DIRS})
target_link_libraries(ruby_version_var PRIVATE ${Ruby_LIBRARIES})
add_test(NAME ruby_version_var COMMAND ruby_version_var)

add_executable(ruby_version_tgt "${CMAKE_CURRENT_LIST_DIR}/ruby_version.c")
target_link_libraries(ruby_version_tgt Ruby::Ruby)
add_test(NAME ruby_version_tgt COMMAND ruby_version_tgt)

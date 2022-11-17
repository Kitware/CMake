enable_language(C)

# Look for a source tree left by enable_language internal checks.
set(scratch ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CMakeScratch)
file(GLOB_RECURSE remnants ${scratch}/TryCompile-*/*)
if(remnants)
  message(FATAL_ERROR "try_compile should not leave artifacts behind")
endif()

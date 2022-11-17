enable_language(C)

# Look for a source tree left by enable_language internal checks.
set(scratch ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CMakeScratch)
file(GLOB_RECURSE remnants ${scratch}/TryCompile-*/CMakeLists.txt)
if(NOT remnants)
  message(FATAL_ERROR "--debug-trycompile should leave the source behind")
endif()

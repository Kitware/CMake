# No languages are enabled by the enclosing project.
add_library(headers INTERFACE)
target_include_directories(headers INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/cxx>"
  "$<$<COMPILE_LANGUAGE:NONE>:${CMAKE_CURRENT_SOURCE_DIR}/none>"
  "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:${CMAKE_CURRENT_SOURCE_DIR}/gnu>")

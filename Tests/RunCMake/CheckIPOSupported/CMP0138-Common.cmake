enable_language(C)
string(APPEND CMAKE_C_FLAGS " -DFOO")
string(APPEND CMAKE_C_FLAGS_DEBUG " -DBAR")
check_ipo_supported(RESULT ipo_supported)
file(STRINGS "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/_CMakeLTOTest-C/bin/CMakeCache.txt"
  cached_flags REGEX "^CMAKE_C_FLAGS(_DEBUG)?:")
foreach(line IN LISTS cached_flags)
  message(STATUS "${line}")
endforeach()

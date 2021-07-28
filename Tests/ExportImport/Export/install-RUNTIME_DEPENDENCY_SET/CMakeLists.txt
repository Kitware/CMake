cmake_minimum_required(VERSION 3.20)

set(CMAKE_SKIP_RPATH OFF)

foreach(i 1 2 3 4 5 6 7 8 9 10 11 12)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/dep${i}.c"
"#ifdef _WIN32
__declspec(dllexport)
#endif
  void dep${i}(void)
{
}
")
  add_library(dep${i} SHARED "${CMAKE_CURRENT_BINARY_DIR}/dep${i}.c")
endforeach()

set_target_properties(dep9 PROPERTIES
  FRAMEWORK TRUE
  )
set_target_properties(dep2 PROPERTIES
  VERSION 1.2.3
  SOVERSION 1
  )

add_library(deplib SHARED deplib.c)
target_link_libraries(deplib PRIVATE dep1)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set_target_properties(deplib PROPERTIES
    INSTALL_RPATH "@loader_path/"
    )
endif()

install(TARGETS dep1 dep2 dep3 dep4 dep5 dep6 dep7 dep8 dep9 dep10 dep11 dep12 deplib EXPORT install-RUNTIME_DEPENDENCY_SET
  RUNTIME DESTINATION install-RUNTIME_DEPENDENCY_SET/bin
  LIBRARY DESTINATION install-RUNTIME_DEPENDENCY_SET/lib
  ARCHIVE DESTINATION install-RUNTIME_DEPENDENCY_SET/lib
  FRAMEWORK DESTINATION install-RUNTIME_DEPENDENCY_SET/frameworks
  )
install(EXPORT install-RUNTIME_DEPENDENCY_SET
  FILE targets.cmake
  DESTINATION install-RUNTIME_DEPENDENCY_SET
  )

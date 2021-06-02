set(CMAKE_SKIP_RPATH OFF)

# Import targets from the install tree.
include(${Import_BINARY_DIR}/../Root/install-RUNTIME_DEPENDENCY_SET/targets.cmake)

add_executable(exe1 main.c)
add_executable(exe2 main.c)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set_target_properties(exe1 exe2 PROPERTIES
    # Multiple MACOSX_BUNDLE executables are allowed on non-macOS platforms.
    MACOSX_BUNDLE TRUE
    )
endif()

add_library(sublib1 SHARED sublib1.c)
target_link_libraries(sublib1 PRIVATE dep6)

add_library(sublib2 SHARED sublib2.c)
target_link_libraries(sublib2 PRIVATE dep7)

foreach(i exe1 exe2)
  target_link_libraries(${i} PRIVATE
    dep1
    dep2
    dep3
    dep4
    dep5
    dep10
    dep11
    dep12
    sublib1
    sublib2
    )
endforeach()

add_library(lib SHARED lib.c)
target_link_libraries(lib PRIVATE dep8)

add_library(mod MODULE mod.c)
target_link_libraries(mod PRIVATE dep9)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set_target_properties(mod PROPERTIES
    SKIP_BUILD_RPATH TRUE
    )
endif()

set(_framework_args)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(_framework_args FRAMEWORK DESTINATION subdir/frameworks)
endif()
install(TARGETS exe1 exe2 lib mod sublib1
  RUNTIME_DEPENDENCIES
    PRE_INCLUDE_REGEXES "$<1:dep([2-9]|1[012])>"
    PRE_EXCLUDE_REGEXES "$<1:.*>"
    POST_INCLUDE_REGEXES "$<1:(bin|lib)/(lib)?dep3>"
    POST_EXCLUDE_REGEXES "$<1:(bin|lib)/(lib)?dep[34]>"
    POST_INCLUDE_FILES "$<TARGET_FILE:dep10>" "$<TARGET_FILE:dep11>"
    POST_EXCLUDE_FILES "$<TARGET_FILE:dep11>" "$<TARGET_FILE:dep12>"
    DIRECTORIES "$<TARGET_FILE_DIR:dep9>"
  RUNTIME DESTINATION "$<1:subdir/bin>"
  LIBRARY DESTINATION "$<1:subdir/lib>"
  ${_framework_args}
  )

install(TARGETS lib
  RUNTIME_DEPENDENCIES
    PRE_INCLUDE_REGEXES dep8
    PRE_EXCLUDE_REGEXES ".*"
    DIRECTORIES "$<TARGET_FILE_DIR:dep8>"
  ${_framework_args}
  )

install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/check_installed.cmake")

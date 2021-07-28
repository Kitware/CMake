set(CMAKE_SKIP_RPATH OFF)

# Import targets from the install tree.
include(${Import_BINARY_DIR}/../Root/install-RUNTIME_DEPENDENCY_SET/targets.cmake)

add_executable(exe main.c)
target_link_libraries(exe PRIVATE dep3 dep4)

install(TARGETS exe RUNTIME_DEPENDENCY_SET myset)
install(IMPORTED_RUNTIME_ARTIFACTS deplib RUNTIME_DEPENDENCY_SET myset)

install(RUNTIME_DEPENDENCY_SET myset
  PRE_INCLUDE_REGEXES "dep[134]"
  PRE_EXCLUDE_REGEXES ".*"
  POST_INCLUDE_REGEXES "dep[13]"
  POST_EXCLUDE_REGEXES "dep[34]"
  DIRECTORIES "$<TARGET_FILE_DIR:dep1>"
  )
install(RUNTIME_DEPENDENCY_SET myset
  PRE_INCLUDE_REGEXES "dep[134]"
  PRE_EXCLUDE_REGEXES ".*"
  DIRECTORIES "$<TARGET_FILE_DIR:dep1>"
  RUNTIME DESTINATION yyy/bin
  LIBRARY DESTINATION yyy/lib
  )
install(RUNTIME_DEPENDENCY_SET myset
  PRE_INCLUDE_REGEXES "dep[134]"
  PRE_EXCLUDE_REGEXES ".*"
  DIRECTORIES "$<TARGET_FILE_DIR:dep1>"
  DESTINATION zzz
  )

install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/check_installed.cmake")

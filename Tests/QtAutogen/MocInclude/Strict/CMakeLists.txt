# Disable relaxed mode
set(CMAKE_AUTOMOC_RELAXED_MODE FALSE)

# Common test
set(COMMON_FUNCTION_NAME commonStrict)
configure_file(
  "${COM_DIR}/common.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/commonStrict.cpp")

makeExecutable(libStrict)
target_sources(libStrict PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/commonStrict.cpp"
  strict.cpp
  )

# Enable relaxed mode
set(CMAKE_AUTOMOC_RELAXED_MODE TRUE)

# Common test
set(COMMON_FUNCTION_NAME commonRelaxed)
configure_file(
  "${COM_DIR}/common.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/commonRelaxed.cpp")

makeExecutable(libRelaxed)
target_sources(libRelaxed PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/commonRelaxed.cpp"
  RObjA.cpp
  RObjB.cpp
  RObjC.cpp
  relaxed.cpp
  )

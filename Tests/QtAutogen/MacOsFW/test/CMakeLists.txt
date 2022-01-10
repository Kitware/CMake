include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
include_directories(SYSTEM
  ${Qt${QT_TEST_VERSION}Core_INCLUDE_DIRS}
  ${Qt${QT_TEST_VERSION}Widgets_INCLUDE_DIRS}
)

set(testname AutomocMacosFWLib)
add_executable(${testname} testMacosFWLib.cpp)
set_target_properties(${testname} PROPERTIES AUTOMOC TRUE)
target_link_libraries(${testname} PRIVATE
  Qt${QT_TEST_VERSION}::Core
  Qt${QT_TEST_VERSION}::Widgets
  Qt${QT_TEST_VERSION}::Test
  macos_fw_lib
)

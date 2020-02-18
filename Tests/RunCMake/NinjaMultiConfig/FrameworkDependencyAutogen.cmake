enable_language(C)

set(CMAKE_LINK_DEPENDS_NO_SHARED ON)

set(QT_VERSION_MAJOR 6)
set(fake_moc_path "${CMAKE_CURRENT_BINARY_DIR}/fake_moc")
file(WRITE "${fake_moc_path}" "")

add_library(test1 SHARED simplelib.c)
add_library(test2 SHARED empty.c)
target_link_libraries(test2 test1)

set_target_properties(test1 test2 PROPERTIES
  FRAMEWORK ON
  AUTOMOC ON
  AUTOMOC_EXECUTABLE "${fake_moc_path}"
  )

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
generate_output_files(test1 test2)

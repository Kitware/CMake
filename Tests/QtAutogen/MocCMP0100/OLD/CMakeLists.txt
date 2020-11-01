cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0100 OLD)

# Generate moc files externally.
# If AUTOMOC generates the header moc files as well
# (it should not in OLD behavior), the test will fail with a
# multiple definition error when linking the executable.
qtx_wrap_cpp(mocCMP0100OldMoc ${CSD}/Obj.hh ${CSD}/Obj2.hh)
qtx_generate_moc(${CBD}/Obj.cpp ${CMAKE_CURRENT_BINARY_DIR}/Obj.moc)
qtx_generate_moc(${CBD}/Obj2.cpp ${CMAKE_CURRENT_BINARY_DIR}/Obj2.moc)

# Make sure AUTOGEN file skipping is disabled
set_source_files_properties(
  ${CSD}/Obj.hh
  ${CBD}/Obj.cpp
  ${CSD}/Obj2.hh
  ${CBD}/Obj2.cpp
  PROPERTIES
  SKIP_AUTOGEN OFF
  SKIP_AUTOMOC OFF
)

add_executable(mocCMP0100Old
  ${CSD}/main.cpp
  ${CSD}/Obj.hh # Manually include Obj.hh
  ${CSD}/Obj.cpp
  ${CSD}/Obj2.cpp # Let AUTOMOC detect Obj2.hh
  ${mocCMP0100OldMoc}
)
target_link_libraries(mocCMP0100Old ${QT_LIBRARIES})
target_include_directories(mocCMP0100Old PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

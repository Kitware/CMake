check_target_objects(single_subdir_absolute
  "$<TARGET_OBJECTS:c_objs,SOURCE_FILES:${CMAKE_SOURCE_DIR}/empty2.c>"
  [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/SOURCE_FILES-build((/CMakeFiles)?/c_objs\.dir(/(\.|[a-zA-Z]+))?|/build/c_objs\.build/.*)/empty2(\.c)?\.(o|obj)$]]
  )

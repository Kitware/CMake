enable_language(C)

include(common.cmake)

add_library(c_objs OBJECT EXCLUDE_FROM_ALL empty1.c empty2.c empty3.c)

check_target_objects(all
  $<TARGET_OBJECTS:c_objs>
  [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/all-build((/CMakeFiles)?/c_objs\.dir(/(\.|[a-zA-Z]+))?|/build/c_objs\.build/.*)/empty1(\.c)?\.(o|obj)$]]
  [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/all-build((/CMakeFiles)?/c_objs\.dir(/(\.|[a-zA-Z]+))?|/build/c_objs\.build/.*)/empty2(\.c)?\.(o|obj)$]]
  [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/all-build((/CMakeFiles)?/c_objs\.dir(/(\.|[a-zA-Z]+))?|/build/c_objs\.build/.*)/empty3(\.c)?\.(o|obj)$]]
  )

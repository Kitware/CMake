enable_language(C)

add_library(debug_dep STATIC nothing.c)
set_target_properties(debug_dep PROPERTIES MY_PROP "debug")

add_library(release_dep STATIC nothing.c)
set_target_properties(release_dep PROPERTIES MY_PROP "release")

add_library(mylib STATIC nothing.c)
target_link_libraries(mylib PRIVATE
  $<$<CONFIG:Debug>:debug_dep>
  $<$<CONFIG:Release>:release_dep>
)
set_target_properties(mylib PROPERTIES MY_PROP "mylib")

# Driven with -DCMAKE_BUILD_TYPE=Debug - only debug_dep should be reached
# by the walker, since the $<CONFIG:Release> arm evaluates to empty under
# Debug.
cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  DEFERRED
  FOLLOW_DEPENDENCIES
  NAMED MY_PROP
)

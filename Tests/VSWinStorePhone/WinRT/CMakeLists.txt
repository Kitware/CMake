project(JusticeLeagueWinRT CXX)

# create project
add_library(JusticeLeagueWinRT SHARED
  "${CMAKE_CURRENT_SOURCE_DIR}/Batman.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Batman.h"
)

set_target_properties(JusticeLeagueWinRT PROPERTIES
  VS_WINRT_COMPONENT TRUE
  VS_GLOBAL_ROOTNAMESPACE "JusticeLeagueWinRT"
  OUTPUT_NAME "JusticeLeagueWinRT"
)

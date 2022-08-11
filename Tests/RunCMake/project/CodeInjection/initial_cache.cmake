set(CMAKE_TOOLCHAIN_FILE                 "${CMAKE_CURRENT_LIST_DIR}/passthrough_toolchain_file.cmake" CACHE FILEPATH "")
set(CMAKE_PROJECT_INCLUDE                "${CMAKE_CURRENT_LIST_DIR}/cmake_project_include.cmake" CACHE FILEPATH "")
set(CMAKE_PROJECT_INCLUDE_BEFORE         "${CMAKE_CURRENT_LIST_DIR}/cmake_project_include_before.cmake" CACHE FILEPATH "")
set(CMAKE_PROJECT_SubProj_INCLUDE        "${CMAKE_CURRENT_LIST_DIR}/cmake_project_subproj_include.cmake" CACHE FILEPATH "")
set(CMAKE_PROJECT_SubProj_INCLUDE_BEFORE "${CMAKE_CURRENT_LIST_DIR}/cmake_project_subproj_include_before.cmake" CACHE FILEPATH "")
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES
  "${CMAKE_CURRENT_LIST_DIR}/cmake_project_top_level_includes_1.cmake"
  "${CMAKE_CURRENT_LIST_DIR}/cmake_project_top_level_includes_2.cmake"
  CACHE STRING ""
)

set(CMAKE_TOOLCHAIN_FILE                 "${CMAKE_CURRENT_LIST_DIR}/passthrough_toolchain_file.cmake" CACHE FILEPATH "")
set(CMAKE_PROJECT_INCLUDE                "${CMAKE_CURRENT_LIST_DIR}/cmake_project_includes_1.cmake" CACHE FILEPATH "")
set(CMAKE_PROJECT_INCLUDE_BEFORE         "${CMAKE_CURRENT_LIST_DIR}/cmake_project_includes_before_1.cmake" CACHE FILEPATH "")
set(CMAKE_PROJECT_SubProj_INCLUDE        "${CMAKE_CURRENT_LIST_DIR}/cmake_project_subproj_includes_1.cmake" CACHE FILEPATH "")
set(CMAKE_PROJECT_SubProj_INCLUDE_BEFORE "${CMAKE_CURRENT_LIST_DIR}/cmake_project_subproj_includes_before_1.cmake" CACHE FILEPATH "")
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES
  "${CMAKE_CURRENT_LIST_DIR}/cmake_project_top_level_includes_1.cmake"
  CACHE FILEPATH ""
)

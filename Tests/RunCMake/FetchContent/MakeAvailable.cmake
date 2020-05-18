include(FetchContent)

FetchContent_Declare(
  WithProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/WithProject
)
FetchContent_Declare(
  WithoutProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/WithoutProject
)
FetchContent_Declare(
  ProjectSubdir
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/WithoutProject
  SOURCE_SUBDIR ProjectSubdir
)

# Order is important and will be verified by test output
FetchContent_MakeAvailable(WithProject WithoutProject ProjectSubdir)

get_property(addedWith GLOBAL PROPERTY FetchWithProject SET)
if(NOT addedWith)
  message(SEND_ERROR "Project with top level CMakeLists.txt not added")
endif()
get_property(addedSubdir GLOBAL PROPERTY FetchWithSubProject SET)
if(NOT addedSubdir)
  message(SEND_ERROR "Project with CMakeLists.txt in subdir not added")
endif()

include(${withoutproject_SOURCE_DIR}/confirmMessage.cmake)

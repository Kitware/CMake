include(FetchContent)

FetchContent_Declare(
  WithProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/WithProject
)
FetchContent_Declare(
  WithoutProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/WithoutProject
)

# Order is important and will be verified by test output
FetchContent_MakeAvailable(WithProject WithoutProject)

get_property(addedWith GLOBAL PROPERTY FetchWithProject SET)
if(NOT addedWith)
  message(SEND_ERROR "Subdir with CMakeLists.txt not added")
endif()

include(${withoutproject_SOURCE_DIR}/confirmMessage.cmake)

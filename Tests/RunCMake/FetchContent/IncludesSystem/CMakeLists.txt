project(SystemSub NONE)

FetchContent_Declare(
  SubSub1
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/SubSub1
  SYSTEM
)
FetchContent_Declare(
  SubSub2
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/SubSub2
)

FetchContent_MakeAvailable(SubSub1 SubSub2)

add_library(bar STATIC bar.cpp)

add_library(foo STATIC foo.cpp)
set_target_properties(foo PROPERTIES SYSTEM OFF)

add_executable(zot zot.cpp)

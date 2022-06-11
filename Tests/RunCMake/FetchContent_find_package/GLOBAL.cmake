include(FetchContent)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageFindModules)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE ALWAYS)

set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL TRUE)
FetchContent_Declare(
  GlobalWithArgsKeyword
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FatalIfAdded
  FIND_PACKAGE_ARGS
)
FetchContent_Declare(
  GlobalWithoutArgsKeyword
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FatalIfAdded
)

set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL FALSE)
FetchContent_Declare(
  LocalWithArgsKeyword
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FatalIfAdded
  FIND_PACKAGE_ARGS
)
FetchContent_Declare(
  LocalWithoutArgsKeyword
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FatalIfAdded
)
FetchContent_Declare(
  EventuallyGlobal
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FatalIfAdded
)

add_subdirectory(ChildScope)

if(NOT TARGET GlobalWithArgsKeywordExe)
  message(SEND_ERROR "GlobalWithArgsKeywordExe is not a global target")
endif()
if(NOT TARGET GlobalWithoutArgsKeywordExe)
  message(SEND_ERROR "GlobalWithoutArgsKeywordExe is not a global target")
endif()

if(TARGET LocalWithArgsKeywordExe)
  message(SEND_ERROR "LocalWithArgsKeywordExe is unexpectedly a global target")
endif()
if(TARGET LocalWithoutArgsKeywordExe)
  message(SEND_ERROR "LocalWithoutArgsKeywordExe is unexpectedly a global target")
endif()

if(NOT TARGET EventuallyGlobalExe)
  message(SEND_ERROR "EventuallyGlobalExe is not a global target")
endif()

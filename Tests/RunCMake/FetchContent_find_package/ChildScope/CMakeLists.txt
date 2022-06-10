FetchContent_MakeAvailable(
  GlobalWithArgsKeyword
  GlobalWithoutArgsKeyword
  LocalWithArgsKeyword
  LocalWithoutArgsKeyword
)

# This one was declared without GLOBAL, but should still become global when
# this variable is true at the time FetchContent_MakeAvailable() is called
set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL TRUE)
FetchContent_MakeAvailable(EventuallyGlobal)

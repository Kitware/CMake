set(CMAKE_DEBUG_TARGET_PROPERTIES COMPILE_WRAPPERS LINK_WRAPPERS)
include(common.cmake)
enable_language(C)
example_exe(main.c)

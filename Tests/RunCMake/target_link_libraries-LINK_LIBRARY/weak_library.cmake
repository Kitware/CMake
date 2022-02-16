
enable_language(C)

if(CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
  set(CMAKE_LINK_LIBRARY_USING_weak_library "PATH{-weak_library <LIBRARY>}NAME{LINKER:-weak-l<LIB_ITEM>}")
  set(CMAKE_LINK_LIBRARY_USING_weak_library_SUPPORTED TRUE)
else()
  # feature not yet supported for the other environments
  set(CMAKE_LINK_LIBRARY_USING_whole_library_SUPPORTED FALSE)
endif()

add_library(lib SHARED base.c lib.c unref.c)
set_property(TARGET lib PROPERTY OUTPUT_NAME base)

add_executable(main main.c)
target_link_libraries(main PRIVATE "$<LINK_LIBRARY:weak_library,lib>")

add_executable(main2 main.c)
target_link_directories(main2 PRIVATE "$<TARGET_FILE_DIR:lib>")
target_link_libraries(main2 PRIVATE "$<LINK_LIBRARY:weak_library,base>")

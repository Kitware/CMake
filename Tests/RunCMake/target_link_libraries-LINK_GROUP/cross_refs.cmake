
enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_cross_refs_SUPPORTED TRUE)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU"
  AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
set(CMAKE_C_LINK_GROUP_USING_cross_refs "LINKER:--start-group"
                                        "LINKER:--end-group")
elseif(CMAKE_C_COMPILER_ID STREQUAL "SunPro"
       AND CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  set(CMAKE_C_LINK_GROUP_USING_cross_refs "LINKER:-z,rescan-start"
                                          "LINKER:-z,rescan-end")
else()
  # feature not yet supported for the other environments
  set(CMAKE_C_LINK_GROUP_USING_cross_refs_SUPPORTED FALSE)
endif()

add_library(func1 STATIC func1.c func3.c)
add_library(func2 STATIC func2.c)

add_executable(main main.c)
target_link_libraries(main PRIVATE "$<LINK_GROUP:cross_refs,func1,func2>")

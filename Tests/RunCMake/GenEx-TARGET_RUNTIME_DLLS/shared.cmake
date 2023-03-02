enable_language(C)

add_executable(exe main.c)
add_library(lib1 SHARED lib1.c)
add_library(lib2 SHARED lib2.c)
add_library(lib3 SHARED lib3.c)
if(WIN32 OR CYGWIN)
  set_property(TARGET lib3 PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/SomeSubDir/")
endif()

add_library(static STATIC static.c)
add_library(imported SHARED IMPORTED)
set_property(TARGET imported PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/imported.dll")
set_property(TARGET imported PROPERTY IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/imported.lib")
add_library(imported2 SHARED IMPORTED)
if(NOT WIN32 AND NOT CYGWIN)
  set_property(TARGET imported2 PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/imported2.dll")
endif()
set_property(TARGET imported2 PROPERTY IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/imported2.lib")

target_link_libraries(exe PRIVATE lib1 static imported imported2)
target_link_libraries(lib1 PRIVATE lib2)
target_link_libraries(lib1 INTERFACE lib3)

set(expected_dlls "")
if(WIN32 OR CYGWIN)
  set(expected_dlls
    "$<TARGET_FILE:lib1>"
    "$<TARGET_FILE:imported>"
    "$<TARGET_FILE:lib3>"
    "$<TARGET_FILE:lib2>"
    )
  set(expected_dll_dirs
    "$<PATH:GET_PARENT_PATH,$<TARGET_FILE:lib2>>"
    "$<PATH:GET_PARENT_PATH,$<TARGET_FILE:imported>>"
    "$<PATH:GET_PARENT_PATH,$<TARGET_FILE:lib3>>"
    )
endif()

set(content "check_genex(\"${expected_dlls}\" \"$<TARGET_RUNTIME_DLLS:exe>\")
check_genex(\"${expected_dll_dirs}\" \"$<TARGET_RUNTIME_DLL_DIRS:exe>\")\n")

set(condition)
get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(multi_config)
  set(condition CONDITION "$<CONFIG:Debug>")
endif()
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/dlls.cmake" CONTENT "${content}" ${condition})

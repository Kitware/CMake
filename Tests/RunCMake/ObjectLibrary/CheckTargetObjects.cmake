add_library(StaticLib STATIC a.c)

add_custom_command(TARGET StaticLib POST_BUILD
  VERBATIM
  COMMAND ${CMAKE_COMMAND}
    "-DTARGET_OBJECTS=$<TARGET_OBJECTS:StaticLib>"
    -DEXPECTED_NUM_OBJECTFILES=2
    -P "${CMAKE_CURRENT_SOURCE_DIR}/check_object_files.cmake"
  )

add_library(SharedLib SHARED a.c b.c)
target_compile_definitions(SharedLib PRIVATE REQUIRED)

add_custom_command(TARGET SharedLib POST_BUILD
  VERBATIM
  COMMAND ${CMAKE_COMMAND}
    "-DTARGET_OBJECTS:STRING=$<TARGET_OBJECTS:SharedLib>"
    -DEXPECTED_NUM_OBJECTFILES=2
    -P "${CMAKE_CURRENT_SOURCE_DIR}/check_object_files.cmake"
  )

add_executable(ExecObjs a.c b.c exe.c)
target_compile_definitions(ExecObjs PRIVATE REQUIRED)

add_custom_target(check_exec_objs ALL
  VERBATIM
  COMMAND ${CMAKE_COMMAND}
    "-DTARGET_OBJECTS=$<TARGET_OBJECTS:ExecObjs>"
    -DEXPECTED_NUM_OBJECTFILES=3
    -P "${CMAKE_CURRENT_SOURCE_DIR}/check_object_files.cmake"
  DEPENDS ExecObjs
  )

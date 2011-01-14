project(PerConfig C)

# Targets with per-configuration names.
ADD_LIBRARY(pcStatic STATIC pcStatic.c)
SET_PROPERTY(TARGET pcStatic PROPERTY RELEASE_POSTFIX -opt)
SET_PROPERTY(TARGET pcStatic PROPERTY DEBUG_POSTFIX -dbg)
ADD_LIBRARY(pcShared SHARED pcShared.c)
SET_PROPERTY(TARGET pcShared PROPERTY RELEASE_POSTFIX -opt)
SET_PROPERTY(TARGET pcShared PROPERTY DEBUG_POSTFIX -dbg)
SET_PROPERTY(TARGET pcShared PROPERTY VERSION 1.2)
SET_PROPERTY(TARGET pcShared PROPERTY SOVERSION 3)
IF(UNIX AND NOT CYGWIN)
  SET(soname_file -DpcShared_soname_file=$<TARGET_SONAME_FILE:pcShared>)
ENDIF()
ADD_EXECUTABLE(perconfig perconfig.c)
TARGET_LINK_LIBRARIES(perconfig pcStatic pcShared)
SET_PROPERTY(TARGET perconfig PROPERTY RELEASE_POSTFIX -opt)
SET_PROPERTY(TARGET perconfig PROPERTY DEBUG_POSTFIX -dbg)

SET(PerConfig_COMMAND
  ${CMAKE_COMMAND}
    -Dconfiguration=$<CONFIGURATION>
    -Dperconfig_file_dir=$<TARGET_FILE_DIR:perconfig>
    -Dperconfig_file_name=$<TARGET_FILE_NAME:perconfig>
    -Dperconfig_file=$<TARGET_FILE:perconfig>
    -DpcStatic_file=$<TARGET_FILE:pcStatic>
    -DpcStatic_linker_file=$<TARGET_LINKER_FILE:pcStatic>
    -DpcShared_file=$<TARGET_FILE:pcShared>
    -DpcShared_linker_file=$<TARGET_LINKER_FILE:pcShared>
    ${soname_file}
    -P ${PerConfig_SOURCE_DIR}/perconfig.cmake
    )
SET(PerConfig_COMMAND "${PerConfig_COMMAND}" PARENT_SCOPE)
SET(PerConfig_DEPENDS ${PerConfig_SOURCE_DIR}/perconfig.cmake perconfig pcStatic pcShared)


enable_language(C)

cmake_policy(SET CMP0181 ${CMP0181})

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath=\\\$ORIGIN")

add_library(encode_literal SHARED LinkOptionsLib.c)

add_custom_command(TARGET encode_literal
                   POST_BUILD
                   COMMAND "${CMAKE_COMMAND}" "-Dfile=$<TARGET_FILE:encode_literal>"
                                              "-Drpath=\$ORIGIN"
                                              -P "${CMAKE_CURRENT_SOURCE_DIR}/CheckRPath.cmake"
                    VERBATIM)

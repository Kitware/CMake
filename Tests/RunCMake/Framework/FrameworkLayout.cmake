cmake_minimum_required(VERSION 3.4)
enable_language(C)

add_library(Framework ${FRAMEWORK_TYPE}
            foo.c
            foo.h
            res.txt)
set_target_properties(Framework PROPERTIES
                      FRAMEWORK TRUE
                      PUBLIC_HEADER foo.h
                      RESOURCE "res.txt")

add_custom_command(TARGET Framework POST_BUILD
                   COMMAND /usr/bin/file $<TARGET_FILE:Framework>)

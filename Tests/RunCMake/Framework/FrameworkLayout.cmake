cmake_minimum_required(VERSION 3.4)
enable_language(C)

add_library(Framework ${FRAMEWORK_TYPE}
            foo.c
            foo.h
            res.txt
            flatresource.txt
            deepresource.txt
            some.txt)
set_target_properties(Framework PROPERTIES
                      FRAMEWORK TRUE
                      PUBLIC_HEADER foo.h
                      RESOURCE "res.txt")
set_source_files_properties(flatresource.txt PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(deepresource.txt PROPERTIES MACOSX_PACKAGE_LOCATION Resources/deep)
set_source_files_properties(some.txt PROPERTIES MACOSX_PACKAGE_LOCATION somedir)

add_custom_command(TARGET Framework POST_BUILD
                   COMMAND /usr/bin/file $<TARGET_FILE:Framework>)

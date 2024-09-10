project(TestLib C)
add_library(mySharedLibrary TestLib.c)
set_target_properties(mySharedLibrary PROPERTIES ARCHIVE_OUTPUT_DIRECTORY $<1:lib>)

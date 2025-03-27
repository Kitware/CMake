add_library(SLD TestLib.c)
set_target_properties(SLD PROPERTIES ARCHIVE_OUTPUT_DIRECTORY $<1:lib>)
